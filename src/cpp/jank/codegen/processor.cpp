#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/codegen/escape.hpp>

/* The strategy for codegen to C++ is quite simple. Codegen always happens on a
 * single fn, which generates a single C++ struct. Top-level expressions and
 * REPL expressions are all implicitly wrapped in a fn during analysis. If the
 * jank fn has a nested fn, it becomes a nested struct, since this whole
 * generation works recursively.
 *
 * Analysis lifts constants and vars, so those just become members which are
 * initialized in the ctor.
 *
 * The most interesting part is the translation of expressions into statements,
 * so that something like `(println (if foo bar spam))` can become sane C++.
 * To do this, _every_ nested expression is replaced with a temporary and turned
 * into a statement. When needed, such as with if statements, that temporary
 * is mutated from the then/else branches. In other cases, it's just set
 * directly.
 *
 * That means something like `(println (thing) (if foo bar spam))` will become
 * roughly this C++:
 *
 * ```c++
 * object_ptr thing_result(thing->call());
 * object_ptr if_result;
 * if(foo)
 * { if_result = bar; }
 * else
 * { if_result = spam; }
 * println->call(thing_result, if_result);
 * ```
 *
 * This is optimized by knowing what position every expression in, so trivial expressions used
 * as arguments, for example, don't need to be first stored in temporaries.
 */

namespace jank::codegen
{
  namespace detail
  {
    /* Tail recursive fns generate into a while(true) which mutates the params on each loop.
     * But our runtime requires params to be const&, so we can't mutate them; we need to shadow
     * them. So, for tail recursive fns, we name the params with this suffix and then define
     * the actual param names as mutable locals outside of the while loop. */
    constexpr native_string_view const recur_suffix{ "__recur" };

    /* TODO: Consider making this a virtual fn on object, to return the C++ name. */
    native_string_view gen_constant_type(runtime::object_ptr const o, bool const boxed)
    {
      if(o->as_nil())
      { return "jank::runtime::obj::nil_ptr"; }
      else if(auto const * const d = o->as_boolean())
      { return "jank::runtime::obj::boolean_ptr"; }
      else if(auto const * const d = o->as_integer())
      {
        if(boxed)
        { return "jank::runtime::obj::integer_ptr"; }
        return "jank::native_integer";
      }
      else if(auto const * const d = o->as_real())
      {
        if(boxed)
        { return "jank::runtime::obj::real_ptr"; }
        return "jank::native_real";
      }
      else if(auto const * const d = o->as_symbol())
      { return "jank::runtime::obj::symbol_ptr"; }
      else if(auto const * const d = o->as_keyword())
      { return "jank::runtime::obj::keyword_ptr"; }
      else if(auto const * const d = o->as_string())
      { return "jank::runtime::obj::string_ptr"; }
      else if(auto const * const d = o->as_list())
      { return "jank::runtime::obj::list_ptr"; }
      else if(auto const * const d = o->as_vector())
      { return "jank::runtime::obj::vector_ptr"; }
      else if(auto const * const d = o->as_set())
      { return "jank::runtime::obj::set_ptr"; }
      else if(auto const * const d = o->as_map())
      { return "jank::runtime::obj::map_ptr"; }
      else if(auto const * const d = o->as_var())
      { return "jank::runtime::var_ptr"; }
      else
      { return "jank::runtime::object_ptr"; }
    }

    void gen_constant(runtime::object_ptr const o, fmt::memory_buffer &buffer, bool const boxed)
    {
      if(!boxed)
      {
        o->to_string(buffer);
        return;
      }

      auto inserter(std::back_inserter(buffer));
      if(o->as_nil())
      { format_to(inserter, "jank::runtime::JANK_NIL"); }
      else if(auto const * const d = o->as_boolean())
      { format_to(inserter, "jank::make_box<jank::runtime::obj::boolean>({})", d->data); }
      else if(auto const * const d = o->as_integer())
      { format_to(inserter, "jank::make_box<jank::runtime::obj::integer>({})", d->data); }
      else if(auto const * const d = o->as_real())
      { format_to(inserter, "jank::make_box<jank::runtime::obj::real>({})", d->data); }
      else if(auto const * const d = o->as_symbol())
      {
        format_to
        (
          inserter,
          R"(jank::make_box<jank::runtime::obj::symbol>("{}", "{}"))",
          d->ns,
          d->name
        );
      }
      else if(auto const * const d = o->as_keyword())
      {
        format_to
        (
          inserter,
          R"(__rt_ctx.intern_keyword("{}", "{}", {}))",
          d->sym.ns,
          d->sym.name,
          d->resolved
        );
      }
      else if(auto const * const d = o->as_string())
      {
        format_to
        (
          inserter,
          "jank::make_box<jank::runtime::obj::string>({})",
          escaped(d->data)
        );
      }
      else if(auto const * const d = o->as_list())
      {
        auto ret_tmp(runtime::context::unique_string("vec"));
        format_to
        (inserter, "jank::make_box<jank::runtime::obj::list>(", ret_tmp);
        for(auto const &form : d->data)
        {
          format_to(inserter, ", ");
          gen_constant(form, buffer, true);
        }
        format_to(inserter, ")");
      }
      else if(auto const * const d = o->as_vector())
      {
        auto ret_tmp(runtime::context::unique_string("vec"));
        format_to
        (inserter, "jank::make_box<jank::runtime::obj::vector>(", ret_tmp);
        bool need_comma{};
        for(auto const &form : d->data)
        {
          if(need_comma)
          { format_to(inserter, ", "); }
          gen_constant(form, buffer, true);
          need_comma = true;
        }
        format_to(inserter, ")");
      }
      else
      { std::cerr << "unimplemented constant codegen: " << *o << std::endl; }
    }
  }

  processor::processor
  (
    runtime::context &rt_ctx,
    analyze::expression_ptr const &expr
  )
    : rt_ctx{ rt_ctx },
      root_expr{ expr },
      root_fn{ boost::get<analyze::expr::function<analyze::expression>>(expr->data) },
      struct_name{ runtime::context::unique_string(root_fn.name.unwrap_or("fn")) }
  { }

  processor::processor
  (
    runtime::context &rt_ctx,
    analyze::expr::function<analyze::expression> const &expr
  )
    : rt_ctx{ rt_ctx },
      root_fn{ expr },
      struct_name{ runtime::context::unique_string(root_fn.name.unwrap_or("fn")) }
  { }

  option<native_string> processor::gen
  (
    analyze::expression_ptr const &ex,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const box_needed
  )
  {
    option<native_string> ret;
    boost::apply_visitor
    (
      [this, fn_arity, box_needed, &ret](auto const &typed_ex)
      { ret = gen(typed_ex, fn_arity, box_needed); },
      ex->data
    );
    return ret;
  }

  option<native_string> processor::gen
  (
    analyze::expr::def<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(expr.frame->find_lifted_var(expr.name).unwrap().get());
    auto const &munged_name(runtime::munge(var.native_name.name));
    auto ret_tmp(runtime::context::unique_string(munged_name));

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr.value.is_none())
    { return munged_name; }

    auto const val(gen(expr.value.unwrap(), fn_arity, true).unwrap());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      {
        return fmt::format
        (
          "{}->set_root({})",
          runtime::munge(var.native_name.name),
          val
        );
      }
      case analyze::expression_type::return_statement:
      { format_to(inserter, "return "); }
      case analyze::expression_type::statement:
      {
        format_to
        (
          inserter,
          "{}->set_root({});",
          runtime::munge(var.native_name.name),
          val
        );
        return none;
      }
    }
  }

  option<native_string> processor::gen
  (
    analyze::expr::var_deref<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &,
    bool const
  )
  {
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return fmt::format("{}->get_root()", var.native_name.name); }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        format_to(inserter, "return {}->get_root();", var.native_name.name);
        return none;
      }
      /* Statement of a var deref is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<native_string> processor::gen
  (
    analyze::expr::var_ref<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &,
    bool const
  )
  {
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return var.native_name.name; }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        format_to(inserter, "return {};", var.native_name.name);
        return none;
      }
      /* Statement of a var ref is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  void processor::format_elided_var
  (
    native_string_view const &start,
    native_string_view const &end,
    native_string_view const &ret_tmp,
    native_vector<native_box<analyze::expression>> const &arg_exprs,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const arg_box_needed,
    bool const ret_box_needed
  )
  {
    /* TODO: Assert arg count when we know it. */
    native_vector<native_string> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    { arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap()); }

    auto inserter(std::back_inserter(body_buffer));
    native_string_view ret_box;
    if(ret_box_needed)
    { ret_box = "jank::make_box("; }
    format_to(inserter, "auto const {}({}{}", ret_tmp, ret_box, start);
    bool need_comma{};
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      if(need_comma)
      { format_to(inserter, ", "); }
      format_to(inserter, "{}", arg_tmps[i]);
      need_comma = true;
    }
    format_to(inserter, "{}{});", end, (ret_box_needed ? ")" : ""));
  }

  void processor::format_direct_call
  (
    native_string const &source_tmp,
    native_string_view const &ret_tmp,
    native_vector<native_box<analyze::expression>> const &arg_exprs,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const arg_box_needed
  )
  {
    native_vector<native_string> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    { arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap()); }

    auto inserter(std::back_inserter(body_buffer));
    format_to
    (inserter, "auto const {}({}.call(", ret_tmp, source_tmp);

    bool need_comma{};
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      if(need_comma)
      { format_to(inserter, ", "); }
      format_to(inserter, "{}", arg_tmps[i]);
      need_comma = true;
    }
    format_to(inserter, "));");
  }

  void processor::format_dynamic_call
  (
    native_string const &source_tmp,
    native_string_view const &ret_tmp,
    native_vector<native_box<analyze::expression>> const &arg_exprs,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const arg_box_needed
  )
  {
    native_vector<native_string> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    { arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap()); }

    auto inserter(std::back_inserter(body_buffer));
    format_to
    (inserter, "auto const {}(jank::runtime::dynamic_call({}", ret_tmp, source_tmp);
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    { format_to(inserter, ", {}", arg_tmps[i]); }
    if(arg_tmps.size() > runtime::max_params)
    {
      format_to(inserter, "jank::make_box<jank::runtime::obj::list>(");
      for(size_t i{ runtime::max_params }; i < arg_tmps.size(); ++i)
      { format_to(inserter, ", {}", arg_tmps[i]); }
      format_to(inserter, ")");
    }
    format_to(inserter, "));");
  }

  option<native_string> processor::gen
  (
    analyze::expr::call<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const box_needed
  )
  {
    auto inserter(std::back_inserter(body_buffer));

    auto const ret_tmp(runtime::context::unique_string("call"));
    /* Clojure's codegen actually skips vars for certain calls to clojure.core
     * fns; this is not the same as direct linking, which uses `invokeStatic`
     * instead. Rather, this makes calls to `get` become `RT.get`, calls to `+` become
     * `Numbers.add`, and so on. We do the same thing here. */
    bool elided{};
    if(auto const * const ref = boost::get<analyze::expr::var_deref<analyze::expression>>(&expr.source_expr->data))
    {
      if(ref->qualified_name->ns != "clojure.core")
      { }
      else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "get" }))
      {
        format_elided_var("jank::runtime::get(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
        elided = true;
      }
      else if(expr.arg_exprs.empty())
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "rand" }))
        {
          format_elided_var("jank::runtime::obj::rand(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, box_needed);
          elided = true;
        }
      }
      else if(expr.arg_exprs.size() == 1)
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "print" }))
        {
          format_elided_var("jank::runtime::context::print(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "abs" }))
        {
          format_elided_var("jank::runtime::obj::abs(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "sqrt" }))
        {
          format_elided_var("jank::runtime::obj::sqrt(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "int" }))
        {
          format_elided_var("jank::runtime::obj::to_int(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "seq" }))
        {
          format_elided_var("jank::runtime::seq(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "first" }))
        {
          format_elided_var("jank::runtime::first(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "next" }))
        {
          format_elided_var("jank::runtime::next(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "nil?" }))
        {
          format_elided_var("jank::runtime::is_nil(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "some?" }))
        {
          format_elided_var("jank::runtime::is_some(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, box_needed);
          elided = true;
        }
      }
      else if(expr.arg_exprs.size() == 2)
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "+" }))
        {
          format_elided_var("jank::runtime::obj::add(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "-" }))
        {
          format_elided_var("jank::runtime::obj::sub(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "*" }))
        {
          format_elided_var("jank::runtime::obj::mul(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "/" }))
        {
          format_elided_var("jank::runtime::obj::div(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "<" }))
        {
          format_elided_var("jank::runtime::obj::lt(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "<=" }))
        {
          format_elided_var("jank::runtime::obj::lte(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", ">" }))
        {
          format_elided_var("jank::runtime::obj::gt(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", ">=" }))
        {
          format_elided_var("jank::runtime::obj::gte(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "min" }))
        {
          format_elided_var("jank::runtime::obj::min(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "max" }))
        {
          format_elided_var("jank::runtime::obj::max(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "pow" }))
        {
          format_elided_var("jank::runtime::obj::pow(", ")", ret_tmp, expr.arg_exprs, fn_arity, false, box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "conj" }))
        {
          format_elided_var("jank::runtime::conj(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
          elided = true;
        }
      }
      else if(expr.arg_exprs.size() == 3)
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "assoc" }))
        {
          format_elided_var("jank::runtime::assoc(", ")", ret_tmp, expr.arg_exprs, fn_arity, true, false);
          elided = true;
        }
      }
    }
    else if(auto const * const fn = boost::get<analyze::expr::function<analyze::expression>>(&expr.source_expr->data))
    {
      bool variadic{};
      for(auto const &arity: fn->arities)
      {
        if(arity.fn_ctx->is_variadic)
        { variadic = true; }
      }
      if(!variadic)
      {
        auto const &source_tmp(gen(expr.source_expr, fn_arity, false));
        format_direct_call(source_tmp.unwrap(), ret_tmp, expr.arg_exprs, fn_arity, true);
        elided = true;
      }
    }

    if(!elided)
    {
      auto const &source_tmp(gen(expr.source_expr, fn_arity, false));
      format_dynamic_call(source_tmp.unwrap(), ret_tmp, expr.arg_exprs, fn_arity, true);
    }

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<native_string> processor::gen
  (
    analyze::expr::primitive_literal<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &,
    bool const box_needed
  )
  {
    auto const &constant(expr.frame->find_lifted_constant(expr.data).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      {
        if(!box_needed && constant.unboxed_native_name.is_some())
        { return constant.unboxed_native_name.unwrap().name; }
        return constant.native_name.name;
      }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        format_to(inserter, "return {};", constant.native_name.name);
        return none;
      }
      /* Statement of a literal is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<native_string> processor::gen
  (
    analyze::expr::vector<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    native_vector<native_string> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    { data_tmps.emplace_back(gen(data_expr, fn_arity, true).unwrap()); }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("vec"));
    format_to
    (inserter, "auto const {}(jank::make_box<jank::runtime::obj::vector>(", ret_tmp);
    for(auto it(data_tmps.begin()); it != data_tmps.end();)
    {
      format_to(inserter, "{}", *it);
      if(++it != data_tmps.end())
      { format_to(inserter, ", "); }
    }
    format_to(inserter, "));");

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<native_string> processor::gen
  (
    analyze::expr::map<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    native_vector<std::pair<native_string, native_string>> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    {
      data_tmps.emplace_back
      (gen(data_expr.first, fn_arity, true).unwrap(), gen(data_expr.second, fn_arity, true).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("map"));
    format_to
    (
      inserter,
      "auto const {}(jank::make_box<jank::runtime::obj::map>(jank::runtime::detail::in_place_unique{{}}, jank::make_array_box<object_ptr>(",
      ret_tmp
    );
    bool need_comma{};
    for(auto const &data_tmp : data_tmps)
    {
      if(need_comma)
      { format_to(inserter, ", "); }
      format_to(inserter, "{}", data_tmp.first);
      format_to(inserter, ", {}", data_tmp.second);
      need_comma = true;
    }
    format_to(inserter, "),{}));", data_tmps.size() * 2);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<native_string> processor::gen
  (
    analyze::expr::local_reference const &expr,
    analyze::expr::function_arity<analyze::expression> const &,
    bool const
  )
  {
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return runtime::munge(expr.name->name); }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        format_to(inserter, "return {};", runtime::munge(expr.name->name));
        return none;
      }
      /* Statement of a local ref is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<native_string> processor::gen
  (
    analyze::expr::function<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &,
    bool const box_needed
  )
  {
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ rt_ctx, expr };

    auto header_inserter(std::back_inserter(header_buffer));
    format_to(header_inserter, "{}", prc.declaration_str());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return prc.expression_str(box_needed, false); }
      case analyze::expression_type::return_statement:
      {
        auto body_inserter(std::back_inserter(body_buffer));
        format_to(body_inserter, "return {};", prc.expression_str(box_needed, false));
        return none;
      }
      /* Statement of a fn literal is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<native_string> processor::gen
  (
    analyze::expr::recur<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    auto inserter(std::back_inserter(body_buffer));

    native_vector<native_string> arg_tmps;
    arg_tmps.reserve(expr.arg_exprs.size());
    for(auto const &arg_expr : expr.arg_exprs)
    { arg_tmps.emplace_back(gen(arg_expr, fn_arity, true).unwrap()); }

    auto arg_tmp_it(arg_tmps.begin());
    for(auto const &param : fn_arity.params)
    {
      format_to(inserter, "{} = {};", runtime::munge(param->name), *arg_tmp_it);
      ++arg_tmp_it;
    }
    format_to(inserter, "continue;");
    return none;
  }

  option<native_string> processor::gen
  (
    analyze::expr::let<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("let"));
    format_to(inserter, "object_ptr {}{{ jank::runtime::JANK_NIL }}; {{", ret_tmp);
    for(auto const &pair : expr.pairs)
    {
      auto const &val_tmp(gen(pair.second, fn_arity, true));
      auto const &munged_name(runtime::munge(pair.first->name));
      /* Every binding is wrapped in its own scope, to allow shadowing. */
      format_to(inserter, "{{ auto const {}({});", munged_name, val_tmp.unwrap());
    }

    for(auto it(expr.body.body.begin()); it != expr.body.body.end(); )
    {
      auto const &val_tmp(gen(*it, fn_arity, true));

      /* We ignore all values but the last. */
      if(++it == expr.body.body.end() && val_tmp.is_some())
      { format_to(inserter, "{} = {};", ret_tmp, val_tmp.unwrap()); }
    }
    for(auto const &_ : expr.pairs)
    {
      static_cast<void>(_);
      format_to(inserter, "}}");
    }
    format_to(inserter, "}}");

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<native_string> processor::gen
  (
    analyze::expr::do_<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &arity,
    bool const
  )
  {
    option<native_string> last;
    for(auto const &form : expr.body)
    { last = gen(form, arity, true); }

    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return last; }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        if(last.is_none())
        { format_to(inserter, "return jank::runtime::JANK_NIL;"); }
        else
        { format_to(inserter, "return {};", last.unwrap()); }
        return none;
      }
      /* TODO:correct? */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  /* TODO: An if, in return position, without an else, will not return nil in the else. */
  option<native_string> processor::gen
  (
    analyze::expr::if_<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("if"));
    format_to(inserter, "object_ptr {};", ret_tmp);
    auto const &condition_tmp(gen(expr.condition, fn_arity, false));
    format_to(inserter, "if(jank::runtime::detail::truthy({})) {{", condition_tmp.unwrap());
    auto const &then_tmp(gen(expr.then, fn_arity, true));
    if(then_tmp.is_some())
    { format_to(inserter, "{} = {}; }}", ret_tmp, then_tmp.unwrap()); }
    else
    { format_to(inserter, "}}"); }

    if(expr.else_.is_some())
    {
      format_to(inserter, "else {{");
      auto const &else_tmp(gen(expr.else_.unwrap(), fn_arity, true));
      if(else_tmp.is_some())
      { format_to(inserter, "{} = {}; }}", ret_tmp, else_tmp.unwrap()); }
      else
      { format_to(inserter, "}}"); }
    }

    return ret_tmp;
  }

  option<native_string> processor::gen
  (
    analyze::expr::native_raw<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity,
    bool const
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("native"));

    native_vector<native_string> interpolated_chunk_tmps;
    interpolated_chunk_tmps.reserve((expr.chunks.size() / 2) + 1);
    for(auto const &chunk : expr.chunks)
    {
      auto const * const chunk_expr(boost::get<analyze::expression_ptr>(&chunk));
      if(chunk_expr == nullptr)
      { continue; }
      interpolated_chunk_tmps.emplace_back(gen(*chunk_expr, fn_arity, true).unwrap());
    }

    format_to(inserter, "object_ptr {};", ret_tmp);
    format_to(inserter, "{{ object_ptr __value{{ JANK_NIL }};");
    size_t interpolated_chunk_it{};
    for(auto const &chunk : expr.chunks)
    {
      auto const * const code(boost::get<native_string>(&chunk));
      if(code != nullptr)
      { format_to(inserter, "{}", *code); }
      else
      { format_to(inserter, "{}", interpolated_chunk_tmps[interpolated_chunk_it++]); }
    }
    format_to(inserter, ";{} = __value; }}", ret_tmp);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  native_string processor::declaration_str()
  {
    if(!generated_declaration)
    {
      build_header();
      build_body();
      build_footer();
      generated_declaration = true;
    }

    native_string ret;
    ret.reserve(header_buffer.size() + body_buffer.size() + footer_buffer.size());
    ret += native_string_view{ header_buffer.data(), header_buffer.size() };
    ret += native_string_view{ body_buffer.data(), body_buffer.size() };
    ret += native_string_view{ footer_buffer.data(), footer_buffer.size() };
    //std::cout << ret << std::endl;
    return ret;
  }

  void processor::build_header()
  {
    auto inserter(std::back_inserter(header_buffer));
    format_to
    (
      inserter,
      R"(
        struct {0}
          : jank::runtime::object
          , jank::runtime::behavior::callable
          , jank::runtime::behavior::metadatable
        {{
          jank::runtime::context &__rt_ctx;
      )",
      runtime::munge(struct_name.name)
    );

    /* TODO: Inherit from a jit_function base which has all of this. */
    format_to
    (
      inserter,
      R"(
        jank::native_bool equal(object const &rhs) const final
        {{ return this == &rhs; }}
        jank::native_string to_string() const final
        {{ return "jit function"; }}
        jank::native_integer to_hash() const final
        {{ return reinterpret_cast<jank::native_integer>(this); }}
        jank::runtime::behavior::callable const* as_callable() const final
        {{ return this; }}
        jank::runtime::object_ptr with_meta(jank::runtime::object_ptr const m) const final
        {{
          auto const meta(validate_meta(m));
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
          const_cast<{0}*>(this)->meta = meta;
          return const_cast<{0}*>(this);
        }}
        jank::runtime::behavior::metadatable const* as_metadatable() const final
        {{ return this; }}
      )",
      runtime::munge(struct_name.name)
    );

    for(auto const &arity : root_fn.arities)
    {
      for(auto const &v : arity.frame->lifted_vars)
      {

        format_to
        (
          inserter,
          "jank::runtime::var_ptr const {0};", runtime::munge(v.second.native_name.name)
        );
      }

      for(auto const &v : arity.frame->lifted_constants)
      {
        format_to
        (
          inserter,
          "{} const {};",
          detail::gen_constant_type(v.second.data, true),
          runtime::munge(v.second.native_name.name)
        );

        if(v.second.unboxed_native_name.is_some())
        {
          format_to
          (
            inserter,
            "static constexpr {} const {}{{ ",
            detail::gen_constant_type(v.second.data, false),
            runtime::munge(v.second.unboxed_native_name.unwrap().name)
          );
          detail::gen_constant(v.second.data, header_buffer, false);
          format_to(inserter, "}};");
        }
      }

      /* TODO: More useful types here. */
      for(auto const &v : arity.frame->captures)
      {
        format_to
        (
          inserter,
          "jank::runtime::object_ptr const {0};", runtime::munge(v.first->name)
        );
      }
    }

    format_to
    (
      inserter,
      "{0}(jank::runtime::context &__rt_ctx", runtime::munge(struct_name.name)
    );

    for(auto const &arity : root_fn.arities)
    {
      for(auto const &v : arity.frame->captures)
      {
        /* TODO: More useful types here. */
        format_to
        (
          inserter,
          ", jank::runtime::object_ptr {0}", runtime::munge(v.first->name)
        );
      }
    }

    format_to(inserter, ") : __rt_ctx{{ __rt_ctx }}");

    for(auto const &arity : root_fn.arities)
    {
      for(auto const &v : arity.frame->lifted_vars)
      {
        format_to
        (
          inserter,
          R"(, {0}{{ __rt_ctx.intern_var("{1}", "{2}").expect_ok() }})",
          runtime::munge(v.second.native_name.name),
          v.second.var_name->ns,
          v.second.var_name->name
        );
      }

      for(auto const &v : arity.frame->lifted_constants)
      {
        format_to
        (
          inserter,
          ", {0}{{",
          runtime::munge(v.second.native_name.name)
        );
        detail::gen_constant(v.second.data, header_buffer, true);
        format_to(inserter, "}}");
      }

      for(auto const &v : arity.frame->captures)
      {
        format_to
        (
          inserter,
          ", {0}{{ {0} }}",
          runtime::munge(v.first->name)
        );
      }
    }

    format_to(inserter, "{{ }}");
  }

  void processor::build_body()
  {
    auto inserter(std::back_inserter(body_buffer));

    option<size_t> variadic_arg_position;
    for(auto const &arity : root_fn.arities)
    {
      if(arity.fn_ctx->is_variadic)
      { variadic_arg_position = arity.params.size() - 1; }

      native_string_view recur_suffix;
      if(arity.fn_ctx->is_tail_recursive)
      { recur_suffix = detail::recur_suffix; }

      format_to(inserter, "jank::runtime::object_ptr call(");
      bool param_comma{};
      for(auto const &param : arity.params)
      {
        format_to
        (
          inserter,
          "{} jank::runtime::object_ptr const {}{}",
          (param_comma ? ", " : ""),
          runtime::munge(param->name),
          recur_suffix
        );
        param_comma = true;
      }
      format_to
      (
        inserter,
        R"(
          ) const final {{
          using namespace jank;
          using namespace jank::runtime;
        )"
      );

      if(arity.fn_ctx->is_tail_recursive)
      {
        format_to(inserter, "{{");

        for(auto const &param : arity.params)
        {
          format_to
          (
            inserter,
            "auto {0}({0}{1});",
            runtime::munge(param->name),
            recur_suffix
          );
        }

        format_to
        (
          inserter,
          R"(
            while(true)
            {{
          )"
        );
      }

      for(auto const &form : arity.body.body)
      { gen(form, arity, true); }

      if(arity.body.body.empty())
      { format_to(inserter, "return jank::runtime::JANK_NIL;"); }

      if(arity.fn_ctx->is_tail_recursive)
      { format_to(inserter, "}} }}"); }

      format_to(inserter, "}}");
    }

    if(variadic_arg_position.is_some())
    {
      format_to
      (
        inserter,
        "size_t get_variadic_arg_position() const final{{ return static_cast<size_t>({}); }}",
        variadic_arg_position.unwrap()
      );
    }
  }

  void processor::build_footer()
  {
    auto inserter(std::back_inserter(footer_buffer));
    format_to(inserter, "}};");
  }

  native_string processor::expression_str(bool const box_needed, bool const auto_call)
  {
    if(!generated_expression)
    {
      auto inserter(std::back_inserter(expression_buffer));

      if(auto_call)
      {
        /* TODO: There's a Cling bug here which prevents us from returning the fn object itself,
         * to be called in non-JIT code. If we call it here and return the result, it works fine. */
        auto tmp_name(runtime::context::unique_string());
        format_to
        (
          inserter,
          R"(
            {0} {1}{{ *reinterpret_cast<jank::runtime::context*>({2})
          )",
          runtime::munge(struct_name.name),
          tmp_name,
          fmt::ptr(&rt_ctx)
        );

        for(auto const &arity : root_fn.arities)
        {
          for(auto const &v : arity.frame->captures)
          { format_to(inserter, ", {0}", runtime::munge(v.first->name)); }
        }

        format_to(inserter, "}};");

        format_to
        (
          inserter,
          "{}.call();",
          tmp_name
        );
      }
      else
      {
        native_string_view close = ")";
        if(box_needed)
        {
          format_to
          (
            inserter,
            "jank::make_box<{0}>(std::ref(*reinterpret_cast<jank::runtime::context*>({1}))",
            runtime::munge(struct_name.name),
            fmt::ptr(&rt_ctx)
          );
        }
        else
        {
          format_to
          (
            inserter,
            "{0}{{ std::ref(*reinterpret_cast<jank::runtime::context*>({1}))",
            runtime::munge(struct_name.name),
            fmt::ptr(&rt_ctx)
          );
          close = "}";
        }

        for(auto const &arity : root_fn.arities)
        {
          for(auto const &v : arity.frame->captures)
          { format_to(inserter, ", {0}", runtime::munge(v.first->name)); }
        }

        format_to(inserter, "{}", close);
      }

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }
}
