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
 * object_ptr const &thing_result(thing->call());
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
    constexpr std::string_view const recur_suffix{ "__recur" };

    void gen_constant(runtime::object_ptr const &o, fmt::memory_buffer &buffer)
    {
      auto inserter(std::back_inserter(buffer));
      if(o->as_nil())
      { format_to(inserter, "jank::runtime::JANK_NIL"); }
      else if(auto const * const d = o->as_boolean())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::boolean>({})", d->data); }
      else if(auto const * const d = o->as_integer())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::integer>({})", d->data); }
      else if(auto const * const d = o->as_real())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::real>({})", d->data); }
      else if(auto const * const d = o->as_symbol())
      {
        format_to
        (
          inserter,
          R"(jank::runtime::make_box<jank::runtime::obj::symbol>("{}", "{}"))",
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
          "jank::runtime::make_box<jank::runtime::obj::string>({})",
          escaped(d->data)
        );
      }
      else if(auto const * const d = o->as_list())
      {
        auto ret_tmp(runtime::context::unique_string("vec"));
        format_to
        (inserter, "jank::runtime::obj::list::create(std::in_place", ret_tmp);
        for(auto const &form : d->data)
        {
          format_to(inserter, ", ");
          gen_constant(form, buffer);
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
      struct_name{ runtime::context::unique_string() }
  { }

  processor::processor
  (
    runtime::context &rt_ctx,
    analyze::expr::function<analyze::expression> const &expr
  )
    : rt_ctx{ rt_ctx },
      root_fn{ expr },
      struct_name{ runtime::context::unique_string() }
  { }

  option<std::string> processor::gen
  (
    analyze::expression_ptr const &ex,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    option<std::string> ret;
    boost::apply_visitor
    (
      [this, fn_arity, &ret](auto const &typed_ex)
      { ret = gen(typed_ex, fn_arity); },
      ex->data
    );
    return ret;
  }

  option<std::string> processor::gen
  (
    analyze::expr::def<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(expr.frame->find_lifted_var(expr.name).unwrap().get());
    auto const &munged_name(runtime::munge(var.native_name.name));
    auto ret_tmp(runtime::context::unique_string(munged_name));

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr.value.is_none())
    { return munged_name.data; }

    auto const val(gen(expr.value.unwrap(), fn_arity).unwrap());
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

  option<std::string> processor::gen
  (
    analyze::expr::var_deref<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &
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

  option<std::string> processor::gen
  (
    analyze::expr::var_ref<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &
  )
  {
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return var.native_name.name.data; }
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

  option<std::string> processor::gen
  (
    analyze::expr::call<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    /* It's worth noting that there's extra scope wrapped around the generated
     * arg values. This ensures that the args are only retained for the duration
     * of the call. Otherwise, a fn with a lot of calls could lead to growing
     * memory bloat. */
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("call"));

    auto const &source_tmp(gen(expr.source_expr, fn_arity));
    format_to(inserter, "object_ptr {}; {{", ret_tmp);
    std::vector<std::string> arg_tmps;
    arg_tmps.reserve(expr.arg_exprs.size());
    for(auto const &arg_expr : expr.arg_exprs)
    { arg_tmps.emplace_back(gen(arg_expr, fn_arity).unwrap()); }

    format_to
    (inserter, "{} = jank::runtime::dynamic_call({}", ret_tmp, source_tmp.unwrap());
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    { format_to(inserter, ", {}", arg_tmps[i]); }
    if(arg_tmps.size() > runtime::max_params)
    {
      format_to(inserter, "jank::runtime::obj::list::create(");
      for(size_t i{ runtime::max_params }; i < arg_tmps.size(); ++i)
      { format_to(inserter, ", {}", arg_tmps[i]); }
      format_to(inserter, ")");
    }
    format_to(inserter, "); }}");

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<std::string> processor::gen
  (
    analyze::expr::primitive_literal<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &
  )
  {
    auto const &constant(expr.frame->find_lifted_constant(expr.data).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return constant.native_name.name.data; }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        format_to(inserter, "return {};", constant.native_name.name.data);
        return none;
      }
      /* Statement of a var deref is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<std::string> processor::gen
  (
    analyze::expr::vector<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    std::vector<std::string> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    { data_tmps.emplace_back(gen(data_expr, fn_arity).unwrap()); }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("vec"));
    format_to
    (inserter, "auto const &{}(jank::runtime::make_box<jank::runtime::obj::vector>(", ret_tmp);
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

  option<std::string> processor::gen
  (
    analyze::expr::map<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    std::vector<std::pair<std::string, std::string>> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    {
      data_tmps.emplace_back
      (gen(data_expr.first, fn_arity).unwrap(), gen(data_expr.second, fn_arity).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("map"));
    format_to
    (
      inserter,
      "auto const &{}(jank::runtime::make_box<jank::runtime::obj::map>(std::in_place ",
      ret_tmp
    );
    for(auto const &data_tmp : data_tmps)
    {
      format_to(inserter, ", {}", data_tmp.first);
      format_to(inserter, ", {}", data_tmp.second);
    }
    format_to(inserter, "));");

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<std::string> processor::gen
  (
    analyze::expr::local_reference const &expr,
    analyze::expr::function_arity<analyze::expression> const &
  )
  {
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return runtime::munge(expr.name->name).data; }
      case analyze::expression_type::return_statement:
      {
        auto inserter(std::back_inserter(body_buffer));
        format_to(inserter, "return {};", runtime::munge(expr.name->name).data);
        return none;
      }
      /* Statement of a local ref is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<std::string> processor::gen
  (
    analyze::expr::function<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &
  )
  {
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ rt_ctx, expr };

    auto header_inserter(std::back_inserter(header_buffer));
    format_to(header_inserter, "{}", prc.declaration_str());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
      { return prc.expression_str(false); }
      case analyze::expression_type::return_statement:
      {
        auto body_inserter(std::back_inserter(body_buffer));
        format_to(body_inserter, "return {};", prc.expression_str(false));
        return none;
      }
      /* Statement of a fn literal is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<std::string> processor::gen
  (
    analyze::expr::recur<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    auto inserter(std::back_inserter(body_buffer));

    std::vector<std::string> arg_tmps;
    arg_tmps.reserve(expr.arg_exprs.size());
    for(auto const &arg_expr : expr.arg_exprs)
    { arg_tmps.emplace_back(gen(arg_expr, fn_arity).unwrap()); }

    auto arg_tmp_it(arg_tmps.begin());
    for(auto const &param : fn_arity.params)
    {
      format_to(inserter, "{} = {};", runtime::munge(param->name), *arg_tmp_it);
      ++arg_tmp_it;
    }
    return none;
  }

  option<std::string> processor::gen
  (
    analyze::expr::let<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("let"));
    format_to(inserter, "object_ptr {}{{ jank::runtime::JANK_NIL }}; {{", ret_tmp);
    for(auto const &pair : expr.pairs)
    {
      auto const &val_tmp(gen(pair.second, fn_arity));
      auto const &munged_name(runtime::munge(pair.first->name));
      /* Every binding is wrapped in its own scope, to allow shadowing. */
      format_to(inserter, "{{ object_ptr {}{{ {} }};", munged_name, val_tmp.unwrap());
    }

    for(auto it(expr.body.body.begin()); it != expr.body.body.end(); )
    {
      auto const &val_tmp(gen(*it, fn_arity));

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

  option<std::string> processor::gen
  (
    analyze::expr::do_<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &arity
  )
  {
    option<std::string> last;
    for(auto const &form : expr.body)
    { last = gen(form, arity); }

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
      /* Statement of a fn literal is a nop. */
      case analyze::expression_type::statement:
      { return none; }
    }
  }

  option<std::string> processor::gen
  (
    analyze::expr::if_<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("if"));
    format_to(inserter, "object_ptr {};", ret_tmp);
    auto const &condition_tmp(gen(expr.condition, fn_arity));
    format_to(inserter, "if(jank::runtime::detail::truthy({})) {{", condition_tmp.unwrap());
    auto const &then_tmp(gen(expr.then, fn_arity));
    if(then_tmp.is_some())
    { format_to(inserter, "{} = {}; }}", ret_tmp, then_tmp.unwrap()); }
    else
    { format_to(inserter, "}}"); }

    if(expr.else_.is_some())
    {
      format_to(inserter, "else {{");
      auto const &else_tmp(gen(expr.else_.unwrap(), fn_arity));
      if(else_tmp.is_some())
      { format_to(inserter, "{} = {}; }}", ret_tmp, else_tmp.unwrap()); }
      else
      { format_to(inserter, "}}"); }
    }

    return ret_tmp;
  }

  option<std::string> processor::gen
  (
    analyze::expr::native_raw<analyze::expression> const &expr,
    analyze::expr::function_arity<analyze::expression> const &fn_arity
  )
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("native"));

    std::vector<std::string> interpolated_chunk_tmps;
    interpolated_chunk_tmps.reserve((expr.chunks.size() / 2) + 1);
    for(auto const &chunk : expr.chunks)
    {
      auto const * const chunk_expr(boost::get<analyze::expression_ptr>(&chunk));
      if(chunk_expr == nullptr)
      { continue; }
      interpolated_chunk_tmps.emplace_back(gen(*chunk_expr, fn_arity).unwrap());
    }

    format_to(inserter, "object_ptr {};", ret_tmp);
    format_to(inserter, "{{ object_ptr __value{{ JANK_NIL }};");
    size_t interpolated_chunk_it{};
    for(auto const &chunk : expr.chunks)
    {
      auto const * const code(boost::get<runtime::detail::string_type>(&chunk));
      if(code != nullptr)
      { format_to(inserter, "{}", code->data); }
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

  std::string processor::declaration_str()
  {
    if(!generated_declaration)
    {
      build_header();
      build_body();
      build_footer();
      generated_declaration = true;
    }

    std::string ret;
    ret.reserve(header_buffer.size() + body_buffer.size() + footer_buffer.size());
    ret += std::string_view{ header_buffer.data(), header_buffer.size() };
    ret += std::string_view{ body_buffer.data(), body_buffer.size() };
    ret += std::string_view{ footer_buffer.data(), footer_buffer.size() };
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
          , jank::runtime::pool_item_base<{0}>
          , jank::runtime::behavior::callable
          , jank::runtime::behavior::metadatable
        {{
      )",
      runtime::munge(struct_name.name)
    );

    format_to
    (
      inserter,
      R"(
        jank::runtime::detail::boolean_type equal(object const &rhs) const override
        {{ return this == &rhs; }}
        jank::runtime::detail::string_type to_string() const override
        {{ return "jit function"; }}
        jank::runtime::detail::integer_type to_hash() const override
        {{ return reinterpret_cast<jank::runtime::detail::integer_type>(this); }}
        jank::runtime::behavior::callable const* as_callable() const override
        {{ return this; }}
        jank::runtime::object_ptr with_meta(jank::runtime::object_ptr const &m) const override
        {{
          validate_meta(m);
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
          const_cast<{}*>(this)->meta = m;
          return ptr_from_this();
        }}
        jank::runtime::behavior::metadatable const* as_metadatable() const override
        {{ return this; }}
      )",
      runtime::munge(struct_name.name)
    );

    bool needs_member_init{};

    for(auto const &arity : root_fn.arities)
    {
      needs_member_init |= !arity.frame->lifted_vars.empty()
                           || !arity.frame->lifted_constants.empty()
                           || !arity.frame->captures.empty();
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
          "jank::runtime::object_ptr const {0};", runtime::munge(v.second.native_name.name)
        );
      }

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
        format_to
        (
          inserter,
          ", jank::runtime::object_ptr const &{0}", runtime::munge(v.first->name)
        );
      }
    }

    format_to(inserter, ")");

    if(needs_member_init)
    { format_to(inserter, " : "); }

    bool need_member_init_comma{};
    for(auto const &arity : root_fn.arities)
    {
      for(auto const &v : arity.frame->lifted_vars)
      {
        format_to
        (
          inserter,
          R"({0}{1}{{ __rt_ctx.intern_var("{2}", "{3}").expect_ok() }})",
          (need_member_init_comma ? "," : ""),
          runtime::munge(v.second.native_name.name),
          v.second.var_name->ns,
          v.second.var_name->name
        );
        need_member_init_comma = true;
      }

      for(auto const &v : arity.frame->lifted_constants)
      {
        format_to
        (
          inserter,
          "{0}{1}{{",
          (need_member_init_comma ? "," : ""),
          runtime::munge(v.second.native_name.name)
        );
        detail::gen_constant(v.second.data, header_buffer);
        format_to(inserter, "}}");
        need_member_init_comma = true;
      }

      for(auto const &v : arity.frame->captures)
      {
        format_to
        (
          inserter,
          "{0}{1}{{ {1} }}",
          (need_member_init_comma ? "," : ""),
          runtime::munge(v.first->name)
        );
        need_member_init_comma = true;
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

      std::string_view recur_suffix;
      if(arity.fn_ctx->is_tail_recursive)
      { recur_suffix = detail::recur_suffix; }

      format_to(inserter, "jank::runtime::object_ptr call(");
      bool param_comma{};
      for(auto const &param : arity.params)
      {
        format_to
        (
          inserter,
          "{} jank::runtime::object_ptr const &{}{}",
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
          ) const override {{
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
            "jank::runtime::object_ptr {0}{{ {0}{1} }};",
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
      { gen(form, arity); }

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
        "jank::option<size_t> get_variadic_arg_position() const override{{ return {}; }}",
        variadic_arg_position.unwrap()
      );
    }
  }

  void processor::build_footer()
  {
    auto inserter(std::back_inserter(footer_buffer));
    format_to(inserter, "}};");
  }

  /* TODO: Something better than a default arg. */
  std::string processor::expression_str(bool const auto_call)
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
        format_to
        (
          inserter,
          "jank::runtime::make_box<{0}>(std::ref(*reinterpret_cast<jank::runtime::context*>({1}))",
          runtime::munge(struct_name.name),
          fmt::ptr(&rt_ctx)
        );

        for(auto const &arity : root_fn.arities)
        {
          for(auto const &v : arity.frame->captures)
          { format_to(inserter, ", {0}", runtime::munge(v.first->name)); }
        }

        format_to(inserter, ")");
      }

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }
}
