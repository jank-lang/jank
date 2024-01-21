#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/codegen/escape.hpp>
#include <jank/detail/to_runtime_data.hpp>

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
 *
 * Lastly, this is complicated by tracking boxing requirements so that not everything is an
 * `object_ptr`. Judicious use of `auto` and semantic analysis alows us to track when unboxing
 * is supported, although we very rarely know for certain if something is unboxed. We usually
 * only know if it _could_ be.
 */

namespace jank::codegen
{
  namespace detail
  {
    /* Tail recursive fns generate into a while(true) which mutates the params on each loop.
     * But our runtime requires params to be const&, so we can't mutate them; we need to shadow
     * them. So, for tail recursive fns, we name the params with this suffix and then define
     * the actual param names as mutable locals outside of the while loop. */
    constexpr native_persistent_string_view const recur_suffix{ "__recur" };

    /* TODO: Consider making this a on the typed object: the C++ name. */
    native_persistent_string_view
    gen_constant_type(runtime::object_ptr const o, native_bool const boxed)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
      switch(o->type)
      {
        case jank::runtime::object_type::nil:
          {
            return "jank::runtime::obj::nil_ptr";
          }
        case jank::runtime::object_type::boolean:
          {
            return "jank::runtime::obj::boolean_ptr";
          }
        case jank::runtime::object_type::integer:
          {
            if(boxed)
            {
              return "jank::runtime::obj::integer_ptr";
            }
            return "jank::native_integer";
          }
        case jank::runtime::object_type::real:
          {
            if(boxed)
            {
              return "jank::runtime::obj::real_ptr";
            }
            return "jank::native_real";
          }
        case jank::runtime::object_type::symbol:
          {
            return "jank::runtime::obj::symbol_ptr";
          }
        case jank::runtime::object_type::keyword:
          {
            return "jank::runtime::obj::keyword_ptr";
          }
        case jank::runtime::object_type::persistent_string:
          {
            return "jank::runtime::obj::persistent_string_ptr";
          }
        case jank::runtime::object_type::persistent_list:
          {
            return "jank::runtime::obj::persistent_list_ptr";
          }
        case jank::runtime::object_type::persistent_vector:
          {
            return "jank::runtime::obj::persistent_vector_ptr";
          }
        case jank::runtime::object_type::persistent_set:
          {
            return "jank::runtime::obj::persistent_set_ptr";
          }
        case jank::runtime::object_type::persistent_array_map:
          {
            return "jank::runtime::obj::persistent_array_map_ptr";
          }
        case jank::runtime::object_type::var:
          {
            return "jank::runtime::var_ptr";
          }
        default:
          {
            return "jank::runtime::object_ptr";
          }
      }
#pragma clang diagnostic pop
    }

    void
    gen_constant(runtime::object_ptr const o, fmt::memory_buffer &buffer, native_bool const boxed)
    {
      if(!boxed)
      {
        runtime::detail::to_string(o, buffer);
        return;
      }

      auto inserter(std::back_inserter(buffer));

      runtime::visit_object(
        [&](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(std::same_as<T, runtime::obj::nil>)
          {
            fmt::format_to(inserter, "jank::runtime::obj::nil::nil_const()");
          }
          else if constexpr(std::same_as<T, runtime::obj::boolean>)
          /* TODO: Use a constant here. */
          {
            fmt::format_to(inserter,
                           "jank::make_box<jank::runtime::obj::boolean>({})",
                           typed_o->data);
          }
          else if constexpr(std::same_as<T, runtime::obj::integer>)
          {
            fmt::format_to(inserter,
                           "jank::make_box<jank::runtime::obj::integer>({})",
                           typed_o->data);
          }
          else if constexpr(std::same_as<T, runtime::obj::real>)
          {
            fmt::format_to(inserter, "jank::make_box<jank::runtime::obj::real>({})", typed_o->data);
          }
          else if constexpr(std::same_as<T, runtime::obj::symbol>)
          {
            fmt::format_to(inserter,
                           R"(jank::make_box<jank::runtime::obj::symbol>("{}", "{}"))",
                           typed_o->ns,
                           typed_o->name);
          }
          else if constexpr(std::same_as<T, runtime::obj::keyword>)
          {
            fmt::format_to(inserter,
                           R"(__rt_ctx.intern_keyword("{}", "{}", true).expect_ok())",
                           typed_o->sym.ns,
                           typed_o->sym.name);
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_string>)
          {
            fmt::format_to(inserter,
                           "jank::make_box<jank::runtime::obj::persistent_string>({})",
                           escaped(typed_o->data));
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_vector>)
          {
            fmt::format_to(inserter, "jank::make_box<jank::runtime::obj::persistent_vector>(");
            native_bool need_comma{};
            for(auto const &form : typed_o->data)
            {
              if(need_comma)
              {
                fmt::format_to(inserter, ", ");
              }
              need_comma = true;
              gen_constant(form, buffer, true);
            }
            fmt::format_to(inserter, ")");
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_list>)
          {
            fmt::format_to(inserter, "jank::make_box<jank::runtime::obj::persistent_list>(");
            native_bool need_comma{};
            for(auto const &form : typed_o->data)
            {
              if(need_comma)
              {
                fmt::format_to(inserter, ", ");
              }
              need_comma = true;
              gen_constant(form, buffer, true);
            }
            fmt::format_to(inserter, ")");
          }
          /* Cons, etc. */
          else if constexpr(runtime::behavior::seqable<T>)
          {
            fmt::format_to(inserter, "jank::make_box<jank::runtime::obj::persistent_list>(");
            native_bool need_comma{};
            for(auto it(typed_o->fresh_seq()); it != nullptr; it = it->next_in_place())
            {
              if(need_comma)
              {
                fmt::format_to(inserter, ", ");
              }
              need_comma = true;
              gen_constant(it->first(), buffer, true);
            }
            fmt::format_to(inserter, ")");
          }
          else
          {
            throw std::runtime_error{ fmt::format("unimplemented constant codegen: {}\n",
                                                  typed_o->to_string()) };
          }
        },
        o);
    }

    native_persistent_string boxed_local_name(native_persistent_string const &local_name)
    {
      return local_name + "__boxed";
    }
  }

  handle::handle(native_persistent_string const &name, native_bool const boxed)
  {
    if(boxed)
    {
      boxed_name = name;
      unboxed_name = boxed_name;
    }
    else
    {
      unboxed_name = name;
      boxed_name = fmt::format("jank::make_box({})", unboxed_name);
    }
  }

  handle::handle(native_persistent_string const &boxed_name)
    : boxed_name{ boxed_name }
    , unboxed_name{ boxed_name }
  {
  }

  handle::handle(native_persistent_string const &boxed_name,
                 native_persistent_string const &unboxed_name)
    : boxed_name{ boxed_name }
    , unboxed_name{ unboxed_name }
  {
    if(this->boxed_name.empty())
    {
      this->boxed_name = fmt::format("jank::make_box({})", unboxed_name);
    }
  }

  handle::handle(analyze::local_binding const &binding)
  {
    if(binding.needs_box)
    {
      boxed_name = runtime::munge(binding.name->name);
      unboxed_name = boxed_name;
    }
    else if(binding.has_boxed_usage)
    {
      unboxed_name = runtime::munge(binding.name->name);
      boxed_name = detail::boxed_local_name(unboxed_name);
    }
    else
    {
      unboxed_name = runtime::munge(binding.name->name);
    }
  }

  native_persistent_string handle::str(native_bool const needs_box) const
  {
    if(needs_box)
    {
      if(boxed_name.empty())
      {
        throw std::runtime_error{ fmt::format("Missing boxed name for handle {}", unboxed_name) };
      }
      return boxed_name;
    }
    else
    {
      return unboxed_name;
    }
  }

  processor::processor(runtime::context &rt_ctx,
                       analyze::expression_ptr const &expr,
                       native_persistent_string_view const &module,
                       compilation_target const target)
    : rt_ctx{ rt_ctx }
    , root_expr{ expr }
    , root_fn{ boost::get<analyze::expr::function<analyze::expression>>(expr->data) }
    , module{ module }
    , target{ target }
    , struct_name{ root_fn.name }
  {
    assert(root_fn.frame.data);
  }

  processor::processor(runtime::context &rt_ctx,
                       analyze::expr::function<analyze::expression> const &expr,
                       native_persistent_string_view const &module,
                       compilation_target const target)
    : rt_ctx{ rt_ctx }
    , root_fn{ expr }
    , module{ module }
    , target{ target }
    , struct_name{ root_fn.name }
  {
    assert(root_fn.frame.data);
  }

  option<handle> processor::gen(analyze::expression_ptr const &ex,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const box_needed)
  {
    option<handle> ret;
    boost::apply_visitor([this, fn_arity, box_needed, &ret](
                           auto const &typed_ex) { ret = gen(typed_ex, fn_arity, box_needed); },
                         ex->data);
    return ret;
  }

  option<handle> processor::gen(analyze::expr::def<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(expr.frame->find_lifted_var(expr.name).unwrap().get());
    auto const &munged_name(runtime::munge(var.native_name.name));
    auto ret_tmp(runtime::context::unique_string(munged_name));

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr.value.is_none())
    {
      return munged_name;
    }

    auto const val(gen(expr.value.unwrap(), fn_arity, true).unwrap());
    switch(expr.expr_type)
    {
      case analyze::expression_type::expression:
        {
          return fmt::format("{}->bind_root({})",
                             runtime::munge(var.native_name.name),
                             val.str(true));
        }
      case analyze::expression_type::return_statement:
        {
          fmt::format_to(inserter, "return ");
        }
      case analyze::expression_type::statement:
        {
          fmt::format_to(inserter,
                         "{}->bind_root({});",
                         runtime::munge(var.native_name.name),
                         val.str(true));
          return none;
        }
    }
  }

  option<handle> processor::gen(analyze::expr::var_deref<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &,
                                native_bool const)
  {
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::expression:
        {
          return fmt::format("{}->deref()", var.native_name.name);
        }
      case analyze::expression_type::return_statement:
        {
          auto inserter(std::back_inserter(body_buffer));
          fmt::format_to(inserter, "return {}->deref();", var.native_name.name);
          return none;
        }
    }
  }

  option<handle> processor::gen(analyze::expr::var_ref<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &,
                                native_bool const)
  {
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::expression:
        {
          return var.native_name.name;
        }
      case analyze::expression_type::return_statement:
        {
          auto inserter(std::back_inserter(body_buffer));
          fmt::format_to(inserter, "return {};", var.native_name.name);
          return none;
        }
    }
  }

  void
  processor::format_elided_var(native_persistent_string_view const &start,
                               native_persistent_string_view const &end,
                               native_persistent_string_view const &ret_tmp,
                               native_vector<native_box<analyze::expression>> const &arg_exprs,
                               analyze::expr::function_arity<analyze::expression> const &fn_arity,
                               native_bool const arg_box_needed,
                               native_bool const ret_box_needed)
  {
    /* TODO: Assert arg count when we know it. */
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    native_persistent_string_view ret_box;
    if(ret_box_needed)
    {
      ret_box = "jank::make_box(";
    }
    fmt::format_to(inserter, "auto const {}({}{}", ret_tmp, ret_box, start);
    native_bool need_comma{};
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      if(need_comma)
      {
        fmt::format_to(inserter, ", ");
      }
      fmt::format_to(inserter, "{}", arg_tmps[i].str(arg_box_needed));
      need_comma = true;
    }
    fmt::format_to(inserter, "{}{});", end, (ret_box_needed ? ")" : ""));
  }

  void
  processor::format_direct_call(native_persistent_string const &source_tmp,
                                native_persistent_string_view const &ret_tmp,
                                native_vector<native_box<analyze::expression>> const &arg_exprs,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const arg_box_needed)
  {
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    fmt::format_to(inserter, "auto const {}({}.call(", ret_tmp, source_tmp);

    native_bool need_comma{};
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      if(need_comma)
      {
        fmt::format_to(inserter, ", ");
      }
      fmt::format_to(inserter, "{}", arg_tmps[i].str(true));
      need_comma = true;
    }
    fmt::format_to(inserter, "));");
  }

  void
  processor::format_dynamic_call(native_persistent_string const &source_tmp,
                                 native_persistent_string_view const &ret_tmp,
                                 native_vector<native_box<analyze::expression>> const &arg_exprs,
                                 analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                 native_bool const arg_box_needed)
  {
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    fmt::format_to(inserter, "auto const {}(jank::runtime::dynamic_call({}", ret_tmp, source_tmp);
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      fmt::format_to(inserter, ", {}", arg_tmps[i].str(true));
    }

    if(runtime::max_params < arg_tmps.size())
    {
      fmt::format_to(inserter, ", jank::make_box<jank::runtime::obj::persistent_list>(");
      native_bool comma{};
      for(size_t i{ runtime::max_params }; i < arg_tmps.size(); ++i)
      {
        fmt::format_to(inserter, "{} {}", comma ? "," : "", arg_tmps[i].str(true));
        comma = true;
      }
      fmt::format_to(inserter, ")");
    }
    fmt::format_to(inserter, "));");
  }

  option<handle> processor::gen(analyze::expr::call<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const box_needed)
  {
    auto inserter(std::back_inserter(body_buffer));

    /* TODO: Doesn't take into account boxing. */
    handle ret_tmp{ runtime::context::unique_string("call") };
    /* Clojure's codegen actually skips vars for certain calls to clojure.core
     * fns; this is not the same as direct linking, which uses `invokeStatic`
     * instead. Rather, this makes calls to `get` become `RT.get`, calls to `+` become
     * `Numbers.add`, and so on. We do the same thing here. */
    native_bool elided{};
    /* TODO: Use the actual var meta to do this, not a hard-coded set of if checks. */
    if(auto const * const ref
       = boost::get<analyze::expr::var_deref<analyze::expression>>(&expr.source_expr->data))
    {
      if(ref->qualified_name->ns != "clojure.core")
      {
      }
      else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "get" }))
      {
        format_elided_var("jank::runtime::get(",
                          ")",
                          ret_tmp.str(false),
                          expr.arg_exprs,
                          fn_arity,
                          true,
                          false);
        elided = true;
      }
      else if(expr.arg_exprs.empty())
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "rand" }))
        {
          format_elided_var("jank::runtime::rand(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
      }
      else if(expr.arg_exprs.size() == 1)
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "print" }))
        {
          format_elided_var("jank::runtime::context::print(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "abs" }))
        {
          format_elided_var("jank::runtime::abs(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "sqrt" }))
        {
          format_elided_var("jank::runtime::sqrt(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "int" }))
        {
          format_elided_var("jank::runtime::to_int(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "seq" }))
        {
          format_elided_var("jank::runtime::seq(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "fresh_seq" }))
        {
          format_elided_var("jank::runtime::fresh_seq(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "first" }))
        {
          format_elided_var("jank::runtime::first(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "next" }))
        {
          format_elided_var("jank::runtime::next(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "next_in_place" }))
        {
          format_elided_var("jank::runtime::next_in_place(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "nil?" }))
        {
          format_elided_var("jank::runtime::is_nil(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            box_needed);
          elided = true;
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "some?" }))
        {
          format_elided_var("jank::runtime::is_some(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            box_needed);
          elided = true;
        }
      }
      else if(expr.arg_exprs.size() == 2)
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "+" }))
        {
          format_elided_var("jank::runtime::add(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "-" }))
        {
          format_elided_var("jank::runtime::sub(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "*" }))
        {
          format_elided_var("jank::runtime::mul(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "/" }))
        {
          format_elided_var("jank::runtime::div(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "<" }))
        {
          format_elided_var("jank::runtime::lt(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "<=" }))
        {
          format_elided_var("jank::runtime::lte(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", ">" }))
        {
          /* TODO: Use lt and reverse args. */
          format_elided_var("jank::runtime::gt(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", ">=" }))
        {
          /* TODO: Use lte and reverse args. */
          format_elided_var("jank::runtime::gte(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "min" }))
        {
          format_elided_var("jank::runtime::min(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "max" }))
        {
          format_elided_var("jank::runtime::max(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "pow" }))
        {
          format_elided_var("jank::runtime::pow(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "conj" }))
        {
          format_elided_var("jank::runtime::conj(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
      }
      else if(expr.arg_exprs.size() == 3)
      {
        if(ref->qualified_name->equal(runtime::obj::symbol{ "clojure.core", "assoc" }))
        {
          format_elided_var("jank::runtime::assoc(",
                            ")",
                            ret_tmp.str(false),
                            expr.arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
      }
    }
    else if(auto const * const fn
            = boost::get<analyze::expr::function<analyze::expression>>(&expr.source_expr->data))
    {
      native_bool variadic{};
      for(auto const &arity : fn->arities)
      {
        if(arity.fn_ctx->is_variadic)
        {
          variadic = true;
        }
      }
      if(!variadic)
      {
        auto const &source_tmp(gen(expr.source_expr, fn_arity, false));
        format_direct_call(source_tmp.unwrap().str(false),
                           ret_tmp.str(true),
                           expr.arg_exprs,
                           fn_arity,
                           true);
        elided = true;
      }
    }

    if(!elided)
    {
      auto const &source_tmp(gen(expr.source_expr, fn_arity, false));
      format_dynamic_call(source_tmp.unwrap().str(true),
                          ret_tmp.str(true),
                          expr.arg_exprs,
                          fn_arity,
                          true);
    }

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      /* TODO: Box here, not in the calls above. Using false when we mean true is not good. */
      /* No need for extra boxing on this, since the boxing was done on the call above. */
      fmt::format_to(inserter, "return {};", ret_tmp.str(false));
      return none;
    }

    return ret_tmp;
  }

  option<handle> processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &,
                                native_bool const)
  {
    auto const &constant(expr.frame->find_lifted_constant(expr.data).unwrap().get());

    handle ret{ constant.native_name.name };
    if(constant.unboxed_native_name.is_some())
    {
      ret = { constant.native_name.name, constant.unboxed_native_name.unwrap().name };
    }

    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::expression:
        {
          return ret;
        }
      case analyze::expression_type::return_statement:
        {
          auto inserter(std::back_inserter(body_buffer));
          fmt::format_to(inserter, "return {};", ret.str(expr.needs_box));
          return none;
        }
    }
  }

  option<handle> processor::gen(analyze::expr::vector<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    native_vector<handle> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr, fn_arity, true).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("vec"));
    fmt::format_to(inserter, "auto const {}(jank::make_box<jank::runtime::obj::persistent_vector>(", ret_tmp);
    for(auto it(data_tmps.begin()); it != data_tmps.end();)
    {
      fmt::format_to(inserter, "{}", it->str(true));
      if(++it != data_tmps.end())
      {
        fmt::format_to(inserter, ", ");
      }
    }
    fmt::format_to(inserter, "));");

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      fmt::format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<handle> processor::gen(analyze::expr::map<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    native_vector<std::pair<handle, handle>> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr.first, fn_arity, true).unwrap(),
                             gen(data_expr.second, fn_arity, true).unwrap());
    }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("map"));
    /* TODO: Jump right to a hash map, if we have enough values. */
    fmt::format_to(inserter,
                   "auto const "
                   "{}(jank::make_box<jank::runtime::obj::persistent_array_map>(jank::runtime::"
                   "detail::in_place_unique{{}}, jank::make_array_box<object_ptr>(",
                   ret_tmp);
    native_bool need_comma{};
    for(auto const &data_tmp : data_tmps)
    {
      if(need_comma)
      {
        fmt::format_to(inserter, ", ");
      }
      fmt::format_to(inserter, "{}", data_tmp.first.str(true));
      fmt::format_to(inserter, ", {}", data_tmp.second.str(true));
      need_comma = true;
    }
    fmt::format_to(inserter, "),{}));", data_tmps.size() * 2);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      fmt::format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  option<handle> processor::gen(analyze::expr::local_reference const &expr,
                                analyze::expr::function_arity<analyze::expression> const &,
                                native_bool const)
  {
    auto const munged_name(runtime::munge(expr.name->name));

    handle ret;
    if(expr.binding.needs_box)
    {
      ret = munged_name;
    }
    else
    {
      ret = handle{ detail::boxed_local_name(munged_name), munged_name };
    }

    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::expression:
        {
          return ret;
        }
      case analyze::expression_type::return_statement:
        {
          auto inserter(std::back_inserter(body_buffer));
          fmt::format_to(inserter, "return {};", ret.str(expr.needs_box));
          return none;
        }
    }
  }

  option<handle> processor::gen(analyze::expr::function<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &,
                                native_bool const box_needed)
  {
    auto const compiling(runtime::detail::truthy(rt_ctx.compile_files_var->deref()));
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ rt_ctx,
                   expr,
                   runtime::module::nest_module(module, runtime::munge(expr.name)),
                   compiling ? compilation_target::function : compilation_target::repl };

    /* If we're compiling, we'll create a separate file for this. */
    if(target != compilation_target::ns)
    {
      auto header_inserter(std::back_inserter(header_buffer));
      fmt::format_to(header_inserter, "{}", prc.declaration_str());
    }

    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::expression:
        /* TODO: Return a handle. */
        {
          return prc.expression_str(box_needed);
        }
      case analyze::expression_type::return_statement:
        {
          auto body_inserter(std::back_inserter(body_buffer));
          fmt::format_to(body_inserter, "return {};", prc.expression_str(box_needed));
          return none;
        }
    }
  }

  option<handle> processor::gen(analyze::expr::recur<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    auto inserter(std::back_inserter(body_buffer));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr.arg_exprs.size());
    for(auto const &arg_expr : expr.arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, true).unwrap());
    }

    auto arg_tmp_it(arg_tmps.begin());
    for(auto const &param : fn_arity.params)
    {
      fmt::format_to(inserter, "{} = {};", runtime::munge(param->name), arg_tmp_it->str(true));
      ++arg_tmp_it;
    }
    fmt::format_to(inserter, "continue;");
    return none;
  }

  option<handle> processor::gen(analyze::expr::let<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    handle ret_tmp{ runtime::context::unique_string("let"), expr.needs_box };

    if(expr.needs_box)
    {
      fmt::format_to(inserter,
                     "object_ptr {}{{ jank::runtime::obj::nil::nil_const() }}; {{",
                     ret_tmp.str(expr.needs_box));
    }
    else
    {
      fmt::format_to(inserter,
                     "auto const {}([&](){}{{",
                     ret_tmp.str(expr.needs_box),
                     (expr.needs_box ? "-> object_ptr" : ""));
    }

    for(auto const &pair : expr.pairs)
    {
      auto const &val_tmp(gen(pair.second, fn_arity, pair.second->get_base()->needs_box));
      auto const &munged_name(runtime::munge(pair.first->name));
      /* Every binding is wrapped in its own scope, to allow shadowing. */
      fmt::format_to(inserter, "{{ auto const {}({}); ", munged_name, val_tmp.unwrap().str(false));

      auto const local(expr.frame->find_local_or_capture(pair.first));
      if(local.is_none())
      {
        throw std::runtime_error{ fmt::format("ICE: unable to find local: {}",
                                              pair.first->to_string()) };
      }

      auto const &binding(local.unwrap().binding);
      if(!binding.needs_box && binding.has_boxed_usage)
      {
        fmt::format_to(inserter,
                       "auto const {}({});",
                       detail::boxed_local_name(munged_name),
                       val_tmp.unwrap().str(true));
      }
    }

    for(auto it(expr.body.body.begin()); it != expr.body.body.end();)
    {
      auto const &val_tmp(gen(*it, fn_arity, true));

      /* We ignore all values but the last. */
      if(++it == expr.body.body.end() && val_tmp.is_some())
      {
        if(expr.needs_box)
        {
          fmt::format_to(inserter,
                         "{} = {};",
                         ret_tmp.str(true),
                         val_tmp.unwrap().str(expr.needs_box));
        }
        else
        {
          fmt::format_to(inserter, "return {};", val_tmp.unwrap().str(expr.needs_box));
        }
      }
    }
    for(auto const &_ : expr.pairs)
    {
      static_cast<void>(_);
      fmt::format_to(inserter, "}}");
    }

    if(expr.needs_box)
    {
      fmt::format_to(inserter, "}}");
    }
    else
    {
      fmt::format_to(inserter, "}}());");
    }

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      fmt::format_to(inserter, "return {};", ret_tmp.str(expr.needs_box));
      return none;
    }

    return ret_tmp;
  }

  option<handle> processor::gen(analyze::expr::do_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &arity,
                                native_bool const)
  {
    option<handle> last;
    for(auto const &form : expr.body)
    {
      last = gen(form, arity, true);
    }

    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::expression:
        {
          return last;
        }
      case analyze::expression_type::return_statement:
        {
          auto inserter(std::back_inserter(body_buffer));
          if(last.is_none())
          {
            fmt::format_to(inserter, "return jank::runtime::obj::nil::nil_const();");
          }
          else
          {
            fmt::format_to(inserter, "return {};", last.unwrap().str(expr.needs_box));
          }
          return none;
        }
    }
  }

  option<handle> processor::gen(analyze::expr::if_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    /* TODO: Handle unboxed results! */
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("if"));
    fmt::format_to(inserter, "object_ptr {}{{ obj::nil::nil_const() }};", ret_tmp);
    auto const &condition_tmp(gen(expr.condition, fn_arity, false));
    fmt::format_to(inserter,
                   "if(jank::runtime::detail::truthy({})) {{",
                   condition_tmp.unwrap().str(false));
    auto const &then_tmp(gen(expr.then, fn_arity, true));
    if(then_tmp.is_some())
    {
      fmt::format_to(inserter, "{} = {}; }}", ret_tmp, then_tmp.unwrap().str(expr.needs_box));
    }
    else
    {
      fmt::format_to(inserter, "}}");
    }

    if(expr.else_.is_some())
    {
      fmt::format_to(inserter, "else {{");
      auto const &else_tmp(gen(expr.else_.unwrap(), fn_arity, true));
      if(else_tmp.is_some())
      {
        fmt::format_to(inserter, "{} = {}; }}", ret_tmp, else_tmp.unwrap().str(expr.needs_box));
      }
      else
      {
        fmt::format_to(inserter, "}}");
      }
    }
    /* If we don't have an else, but we're in return position, we need to be sure to return
     * something, so we return nil. */
    else if(expr.expr_type == analyze::expression_type::return_statement)
    {
      fmt::format_to(inserter, "else {{ return {}; }}", ret_tmp);
    }

    return ret_tmp;
  }

  option<handle> processor::gen(analyze::expr::throw_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &value_tmp(gen(expr.value, fn_arity, true));
    fmt::format_to(inserter, "throw {};", value_tmp.unwrap().str(true));
    return none;
  }

  option<handle> processor::gen(analyze::expr::native_raw<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity,
                                native_bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(runtime::context::unique_string("native"));

    native_vector<handle> interpolated_chunk_tmps;
    interpolated_chunk_tmps.reserve((expr.chunks.size() / 2) + 1);
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference): Not sure what's up with this.
    for(auto const &chunk : expr.chunks)
    {
      auto const * const chunk_expr(boost::get<analyze::expression_ptr>(&chunk));
      if(chunk_expr == nullptr)
      {
        continue;
      }
      interpolated_chunk_tmps.emplace_back(gen(*chunk_expr, fn_arity, true).unwrap());
    }

    fmt::format_to(inserter, "object_ptr {}{{ obj::nil::nil_const() }};", ret_tmp);
    fmt::format_to(inserter, "{{ object_ptr __value{{ obj::nil::nil_const() }};");
    size_t interpolated_chunk_it{};
    for(auto const &chunk : expr.chunks)
    {
      auto const * const code(boost::get<native_persistent_string>(&chunk));
      if(code != nullptr)
      {
        fmt::format_to(inserter, "{}", *code);
      }
      else
      {
        fmt::format_to(inserter, "{}", interpolated_chunk_tmps[interpolated_chunk_it++].str(true));
      }
    }
    fmt::format_to(inserter, ";{} = __value; }}", ret_tmp);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      fmt::format_to(inserter, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  native_persistent_string processor::declaration_str()
  {
    if(!generated_declaration)
    {
      build_header();
      build_body();
      build_footer();
      generated_declaration = true;
    }

    native_transient_string ret;
    ret.reserve(header_buffer.size() + body_buffer.size() + footer_buffer.size());
    ret += native_persistent_string_view{ header_buffer.data(), header_buffer.size() };
    ret += native_persistent_string_view{ body_buffer.data(), body_buffer.size() };
    ret += native_persistent_string_view{ footer_buffer.data(), footer_buffer.size() };
    //fmt::println("codegen declaration {}", ret);
    return ret;
  }

  void processor::build_header()
  {
    auto inserter(std::back_inserter(header_buffer));

    /* TODO: We don't want this for nested modules, but we do if they're in their own file.
     * Do we need three module compilation targets? Top-level, nested, local?
     *
     * Local fns are within a struct already, so we can't enter the ns again. */
    if(!runtime::module::is_nested_module(module))
    {
      fmt::format_to(inserter, "namespace {} {{", runtime::module::module_to_native_ns(module));
    }

    fmt::format_to(inserter,
                   R"(
        struct {0} : jank::runtime::obj::jit_function
        {{
          jank::runtime::context &__rt_ctx;
      )",
                   runtime::munge(struct_name.name));

    {
      /* TODO: Constants and vars are not shared across arities. We'd need stable names. */
      native_set<native_integer> used_vars, used_constants, used_captures;
      for(auto const &arity : root_fn.arities)
      {
        for(auto const &v : arity.frame->lifted_vars)
        {
          if(used_vars.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_vars.emplace(v.second.native_name.to_hash());

          fmt::format_to(inserter,
                         "jank::runtime::var_ptr const {0};",
                         runtime::munge(v.second.native_name.name));
        }

        for(auto const &v : arity.frame->lifted_constants)
        {
          if(used_constants.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_constants.emplace(v.second.native_name.to_hash());

          fmt::format_to(inserter,
                         "{} const {};",
                         detail::gen_constant_type(v.second.data, true),
                         runtime::munge(v.second.native_name.name));

          if(v.second.unboxed_native_name.is_some())
          {
            fmt::format_to(inserter,
                           "static constexpr {} const {}{{ ",
                           detail::gen_constant_type(v.second.data, false),
                           runtime::munge(v.second.unboxed_native_name.unwrap().name));
            detail::gen_constant(v.second.data, header_buffer, false);
            fmt::format_to(inserter, "}};");
          }
        }

        /* TODO: More useful types here. */
        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          fmt::format_to(inserter,
                         "jank::runtime::object_ptr const {0};",
                         runtime::munge(v.first->name));
        }
      }
    }

    {
      native_set<native_integer> used_captures;
      fmt::format_to(inserter,
                     "{0}(jank::runtime::context &__rt_ctx",
                     runtime::munge(struct_name.name));

      for(auto const &arity : root_fn.arities)
      {
        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          /* TODO: More useful types here. */
          fmt::format_to(inserter,
                         ", jank::runtime::object_ptr {0}",
                         runtime::munge(v.first->name));
        }
      }
    }

    {
      native_set<native_integer> used_vars, used_constants, used_captures;
      fmt::format_to(inserter, ") : __rt_ctx{{ __rt_ctx }}");

      for(auto const &arity : root_fn.arities)
      {
        for(auto const &v : arity.frame->lifted_vars)
        {
          if(used_vars.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_vars.emplace(v.second.native_name.to_hash());

          fmt::format_to(inserter,
                         R"(, {0}{{ __rt_ctx.intern_var("{1}", "{2}").expect_ok() }})",
                         runtime::munge(v.second.native_name.name),
                         v.second.var_name->ns,
                         v.second.var_name->name);
        }

        for(auto const &v : arity.frame->lifted_constants)
        {
          if(used_constants.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_constants.emplace(v.second.native_name.to_hash());

          fmt::format_to(inserter, ", {0}{{", runtime::munge(v.second.native_name.name));
          detail::gen_constant(v.second.data, header_buffer, true);
          fmt::format_to(inserter, "}}");
        }

        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          fmt::format_to(inserter, ", {0}{{ {0} }}", runtime::munge(v.first->name));
        }
      }
    }

    fmt::format_to(inserter, "{{ }}");
  }

  void processor::build_body()
  {
    auto inserter(std::back_inserter(body_buffer));

    analyze::expr::function_arity<analyze::expression> const *variadic_arity{};
    analyze::expr::function_arity<analyze::expression> const *highest_fixed_arity{};
    for(auto const &arity : root_fn.arities)
    {
      if(arity.fn_ctx->is_variadic)
      {
        variadic_arity = &arity;
      }
      else if(!highest_fixed_arity
              || highest_fixed_arity->fn_ctx->param_count < arity.fn_ctx->param_count)
      {
        highest_fixed_arity = &arity;
      }

      native_persistent_string_view recur_suffix;
      if(arity.fn_ctx->is_tail_recursive)
      {
        recur_suffix = detail::recur_suffix;
      }

      fmt::format_to(inserter, "jank::runtime::object_ptr call(");
      native_bool param_comma{};
      for(auto const &param : arity.params)
      {
        fmt::format_to(inserter,
                       "{} jank::runtime::object_ptr const {}{}",
                       (param_comma ? ", " : ""),
                       runtime::munge(param->name),
                       recur_suffix);
        param_comma = true;
      }

      fmt::format_to(inserter,
                     R"(
          ) const final {{
          using namespace jank;
          using namespace jank::runtime;
        )");

      fmt::format_to(inserter, "jank::profile::timer __timer{{ \"{}\" }};", root_fn.name);

      if(arity.fn_ctx->is_tail_recursive)
      {
        fmt::format_to(inserter, "{{");

        for(auto const &param : arity.params)
        {
          fmt::format_to(inserter, "auto {0}({0}{1});", runtime::munge(param->name), recur_suffix);
        }

        fmt::format_to(inserter,
                       R"(
            while(true)
            {{
          )");
      }

      for(auto const &form : arity.body.body)
      {
        gen(form, arity, true);
      }

      if(arity.body.body.empty())
      {
        fmt::format_to(inserter, "return jank::runtime::obj::nil::nil_const();");
      }

      if(arity.fn_ctx->is_tail_recursive)
      {
        fmt::format_to(inserter, "}} }}");
      }

      fmt::format_to(inserter, "}}");
    }

    if(variadic_arity)
    {
      native_bool const variadic_ambiguous{ highest_fixed_arity
                                            && highest_fixed_arity->fn_ctx->param_count
                                              == variadic_arity->fn_ctx->param_count - 1 };

      fmt::format_to(inserter,
                     R"(
          jank::runtime::behavior::callable::arity_flag_t get_arity_flags() const final
          {{ return jank::runtime::behavior::callable::build_arity_flags({}, true, {}); }}
        )",
                     variadic_arity->fn_ctx->param_count - 1,
                     variadic_ambiguous);
    }
  }

  void processor::build_footer()
  {
    auto inserter(std::back_inserter(footer_buffer));

    /* Struct. */
    fmt::format_to(inserter, "}};");

    /* Namespace. */
    if(!runtime::module::is_nested_module(module))
    {
      fmt::format_to(inserter, "}}");
    }
  }

  native_persistent_string processor::expression_str(native_bool const box_needed)
  {
    auto const module_ns(runtime::module::module_to_native_ns(module));

    if(!generated_expression)
    {
      auto inserter(std::back_inserter(expression_buffer));

      native_persistent_string_view close = ").data";
      if(box_needed)
      {
        fmt::format_to(
          inserter,
          "jank::make_box<{0}>(__rt_ctx",
          runtime::module::nest_native_ns(module_ns, runtime::munge(struct_name.name)));
      }
      else
      {
        fmt::format_to(
          inserter,
          "{0}{{ __rt_ctx",
          runtime::module::nest_native_ns(module_ns, runtime::munge(struct_name.name)));
        close = "}";
      }

      native_set<native_integer> used_captures;
      for(auto const &arity : root_fn.arities)
      {
        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          /* We're generating the inputs to the function ctor, which means we don't
            * want the binding of the capture within the function; we want the one outside
            * of it, which we're capturing. We need to reach further for that. */
          auto const originating_local(root_fn.frame->find_local_or_capture(v.first));
          handle h{ originating_local.unwrap().binding };
          fmt::format_to(inserter, ", {0}", h.str(true));
        }
      }

      fmt::format_to(inserter, "{}", close);

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }

  native_persistent_string processor::module_init_str(native_persistent_string_view const &module)
  {
    fmt::memory_buffer module_buffer;
    auto inserter(std::back_inserter(module_buffer));

    fmt::format_to(inserter, "namespace {} {{", runtime::module::module_to_native_ns(module));

    fmt::format_to(inserter,
                   R"(
        struct __ns__init
        {{
      )");

    fmt::format_to(inserter, "static void __init(){{");
    fmt::format_to(inserter, "jank::profile::timer __timer{{ \"ns __init\" }};");
    fmt::format_to(
      inserter,
      "constexpr auto const deps(jank::util::make_array<jank::native_persistent_string_view>(");
    native_bool needs_comma{};
    for(auto const &dep : rt_ctx.module_dependencies[module])
    {
      if(needs_comma)
      {
        fmt::format_to(inserter, ", ");
      }
      fmt::format_to(inserter, "\"{}\"", dep);
      needs_comma = true;
    }
    fmt::format_to(inserter, "));");

    fmt::format_to(inserter, "for(auto const &dep : deps){{");
    fmt::format_to(inserter, "__rt_ctx.load_module(dep);");
    fmt::format_to(inserter, "}}");

    /* __init fn */
    fmt::format_to(inserter, "}}");

    /* Struct */
    fmt::format_to(inserter, "}};");

    /* Namespace */
    fmt::format_to(inserter, "}}");

    native_transient_string ret;
    ret.reserve(module_buffer.size());
    ret += native_persistent_string_view{ module_buffer.data(), module_buffer.size() };
    return ret;
  }
}
