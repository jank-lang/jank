#include <Interpreter/Compatibility.h>
#include <clang/Interpreter/CppInterOp.h>

#include <jank/codegen/processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/util/escape.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/clang_format.hpp>
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
 * object_ref thing_result(thing->call());
 * object_ref if_result;
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
 * `object_ref`. Judicious use of `auto` and semantic analysis alows us to track when unboxing
 * is supported, although we very rarely know for certain if something is unboxed. We usually
 * only know if it _could_ be.
 */

namespace jank::codegen
{
  using namespace jank::analyze;

  namespace detail
  {
    /* Tail recursive fns generate into a while(true) which mutates the params on each loop.
     * But our runtime requires params to be const&, so we can't mutate them; we need to shadow
     * them. So, for tail recursive fns, we name the params with this suffix and then define
     * the actual param names as mutable locals outside of the while loop. */
    constexpr jtl::immutable_string_view const recur_suffix{ "__recur" };

    /* TODO: Consider making this a on the typed object: the C++ name. */
    static jtl::immutable_string gen_constant_type(runtime::object_ref const o, bool const boxed)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
      switch(o->type)
      {
        case jank::runtime::object_type::nil:
          {
            return "jank::runtime::obj::nil_ref";
          }
        case jank::runtime::object_type::boolean:
          {
            return "jank::runtime::obj::boolean_ref";
          }
        case jank::runtime::object_type::integer:
          {
            if(boxed)
            {
              return "jank::runtime::obj::integer_ref";
            }
            return "jank::i64";
          }
        case jank::runtime::object_type::character:
          {
            if(boxed)
            {
              return "jank::runtime::obj::character_ref";
            }
            return "jank::runtime::obj::character";
          }
        case jank::runtime::object_type::real:
          {
            if(boxed)
            {
              return "jank::runtime::obj::real_ref";
            }
            return "jank::f64";
          }
        case jank::runtime::object_type::symbol:
          {
            return "jank::runtime::obj::symbol_ref";
          }
        case jank::runtime::object_type::keyword:
          {
            return "jank::runtime::obj::keyword_ref";
          }
        case jank::runtime::object_type::persistent_string:
          {
            return "jank::runtime::obj::persistent_string_ref";
          }
        case jank::runtime::object_type::persistent_list:
          {
            return "jank::runtime::obj::persistent_list_ref";
          }
        case jank::runtime::object_type::persistent_vector:
          {
            return "jank::runtime::obj::persistent_vector_ref";
          }
        case jank::runtime::object_type::persistent_hash_set:
          {
            return "jank::runtime::obj::persistent_hash_set_ref";
          }
        case jank::runtime::object_type::persistent_array_map:
          {
            return "jank::runtime::obj::persistent_array_map_ref";
          }
        case jank::runtime::object_type::var:
          {
            return "jank::runtime::var_ref";
          }
        default:
          {
            return "jank::runtime::object_ref";
          }
      }
#pragma clang diagnostic pop
    }

    static void
    gen_constant(runtime::object_ref const o, jtl::string_builder &buffer, bool const boxed)
    {
      if(!boxed)
      {
        runtime::to_string(o, buffer);
        return;
      }

      runtime::visit_object(
        [&](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(std::same_as<T, runtime::obj::nil>)
          {
            util::format_to(buffer, "jank::runtime::jank_nil");
          }
          else if constexpr(std::same_as<T, runtime::obj::boolean>)
          {
            if(typed_o->data)
            {
              util::format_to(buffer, "jank::runtime::jank_true");
            }
            else
            {
              util::format_to(buffer, "jank::runtime::jank_false");
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::integer>)
          {
            util::format_to(
              buffer,
              "jank::runtime::make_box<jank::runtime::obj::integer>(static_cast<jank::"
              "i64>({}))",
              typed_o->data);
          }
          else if constexpr(std::same_as<T, runtime::obj::real>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::real>(static_cast<jank::"
                            "f64>({}))",
                            typed_o->data);
          }
          else if constexpr(std::same_as<T, runtime::obj::symbol>)
          {
            if(typed_o->meta.is_some())
            {
              util::format_to(buffer, "jank::runtime::make_box<jank::runtime::obj::symbol>( ");
              gen_constant(typed_o->meta.unwrap(), buffer, true);
              util::format_to(buffer, R"(, "{}", "{}"))", typed_o->ns, typed_o->name);
            }
            else
            {
              util::format_to(buffer,
                              R"(jank::runtime::make_box<jank::runtime::obj::symbol>("{}", "{}"))",
                              typed_o->ns,
                              typed_o->name);
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::character>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::character>({}))",
                            typed_o->to_code_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::keyword>)
          {
            util::format_to(
              buffer,
              R"(jank::runtime::__rt_ctx->intern_keyword("{}", "{}", true).expect_ok())",
              typed_o->sym->ns,
              typed_o->sym->name);
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_string>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::persistent_string>({})",
                            typed_o->to_code_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_vector>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::persistent_vector>(");
            if(typed_o->meta.is_some())
            {
              /* TODO: If meta is empty, use empty() fn. We'll need a gen helper for this. */
              util::format_to(buffer,
                              "jank::runtime::__rt_ctx->read_string(\"{}\"), ",
                              util::escape(runtime::to_code_string(typed_o->meta.unwrap())));
            }
            util::format_to(buffer, "std::in_place ");
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form, buffer, true);
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_list>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::persistent_list>(");
            if(typed_o->meta.is_some())
            {
              util::format_to(buffer,
                              "jank::runtime::__rt_ctx->read_string(\"{}\"), ",
                              util::escape(runtime::to_code_string(typed_o->meta.unwrap())));
            }
            util::format_to(buffer, "std::in_place ");
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form, buffer, true);
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_hash_set>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::persistent_hash_set>(");
            if(typed_o->meta.is_some())
            {
              util::format_to(buffer,
                              "jank::runtime::__rt_ctx->read_string(\"{}\"), ",
                              util::escape(runtime::to_code_string(typed_o->meta.unwrap())));
            }
            util::format_to(buffer, "std::in_place ");
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form, buffer, true);
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_array_map>)
          {
            bool need_comma{};
            if(typed_o->meta.is_some())
            {
              util::format_to(buffer,
                              "jank::runtime::obj::persistent_array_map::create_unique_with_meta(");
              util::format_to(buffer,
                              "jank::runtime::__rt_ctx->read_string(\"{}\")",
                              util::escape(runtime::to_code_string(typed_o->meta.unwrap())));
              need_comma = true;
            }
            else
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_array_map::create_unique(");
            }
            for(auto const &form : typed_o->data)
            {
              if(need_comma)
              {
                util::format_to(buffer, ", ");
              }
              need_comma = true;
              gen_constant(form.first, buffer, true);
              util::format_to(buffer, ", ");
              gen_constant(form.second, buffer, true);
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_hash_map>)
          {
            auto const has_meta{ typed_o->meta.is_some() };
            if(has_meta)
            {
              util::format_to(buffer, "jank::runtime::reset_meta(");
            }
            util::format_to(buffer,
                            "jank::runtime::__rt_ctx->read_string(\"{}\")",
                            util::escape(typed_o->to_code_string()));
            if(has_meta)
            {
              util::format_to(buffer,
                              ", jank::runtime::__rt_ctx->read_string(\"{}\"))",
                              util::escape(runtime::to_code_string(typed_o->meta.unwrap())));
            }
          }
          /* Cons, etc. */
          else if constexpr(runtime::behavior::seqable<T>)
          {
            util::format_to(
              buffer,
              "jank::runtime::make_box<jank::runtime::obj::persistent_list>(std::in_place");
            for(auto it : runtime::make_sequence_range(typed_o))
            {
              util::format_to(buffer, ", ");
              gen_constant(it, buffer, true);
            }
            util::format_to(buffer, ")");
          }
          else
          {
            throw std::runtime_error{ util::format("unimplemented constant codegen: {}\n",
                                                   typed_o->to_string()) };
          }
        },
        o);
    }

    static jtl::immutable_string boxed_local_name(jtl::immutable_string const &local_name)
    {
      return local_name + "__boxed";
    }
  }

  handle::handle(jtl::immutable_string const &name, bool const boxed)
  {
    if(boxed)
    {
      boxed_name = name;
      unboxed_name = boxed_name;
    }
    else
    {
      unboxed_name = name;
      boxed_name = util::format("jank::runtime::make_box({})", unboxed_name);
    }
  }

  handle::handle(jtl::immutable_string const &boxed_name)
    : boxed_name{ boxed_name }
    , unboxed_name{ boxed_name }
  {
  }

  handle::handle(jtl::immutable_string const &boxed_name, jtl::immutable_string const &unboxed_name)
    : boxed_name{ boxed_name }
    , unboxed_name{ unboxed_name }
  {
    if(this->boxed_name.empty())
    {
      this->boxed_name = util::format("jank::runtime::make_box({})", unboxed_name);
    }
  }

  handle::handle(analyze::local_binding_ptr const binding)
  {
    if(binding->needs_box)
    {
      boxed_name = runtime::munge(binding->native_name);
      unboxed_name = boxed_name;
    }
    else if(binding->has_boxed_usage)
    {
      unboxed_name = runtime::munge(binding->native_name);
      boxed_name = detail::boxed_local_name(unboxed_name);
    }
    else
    {
      unboxed_name = runtime::munge(binding->native_name);
    }
  }

  jtl::immutable_string handle::str(bool const needs_box) const
  {
    if(needs_box)
    {
      if(boxed_name.empty())
      {
        throw std::runtime_error{ util::format("Missing boxed name for handle {}", unboxed_name) };
      }
      return boxed_name;
    }
    else
    {
      return unboxed_name;
    }
  }

  processor::processor(analyze::expr::function_ref const expr,
                       jtl::immutable_string const &module,
                       compilation_target const target)
    : root_fn{ expr }
    , module{ module }
    , target{ target }
    , struct_name{ root_fn->unique_name }
  {
    assert(root_fn->frame.data);
  }

  jtl::option<handle> processor::gen(analyze::expression_ref const ex,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const box_needed)
  {
    jtl::option<handle> ret;
    visit_expr([&, this](auto const typed_ex) { ret = gen(typed_ex, fn_arity, box_needed); }, ex);
    return ret;
  }

  jtl::option<handle> processor::gen(analyze::expr::def_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    auto const &var(expr->frame->find_lifted_var(expr->name).unwrap().get());
    auto const &munged_name(runtime::munge(var.native_name));
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string(munged_name)));

    jtl::option<std::reference_wrapper<analyze::lifted_constant const>> meta;
    if(expr->name->meta.is_some())
    {
      meta = expr->frame->find_lifted_constant(expr->name->meta.unwrap()).unwrap();
    }

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr->value.is_none())
    {
      if(meta.is_some())
      {
        auto const dynamic{ truthy(
          get(meta.unwrap().get().data, __rt_ctx->intern_keyword("dynamic").expect_ok())) };
        return util::format("{}->with_meta({})->set_dynamic({})",
                            runtime::munge(var.native_name),
                            runtime::munge(meta.unwrap().get().native_name),
                            dynamic);
      }
      else
      {
        return util::format("{}->with_meta(jank::runtime::jank_nil)",
                            runtime::munge(var.native_name));
      }
    }

    auto const val(gen(expr->value.unwrap(), fn_arity, true).unwrap());
    switch(expr->position)
    {
      case analyze::expression_position::value:
        {
          if(meta.is_some())
          {
            auto const dynamic{ truthy(
              get(meta.unwrap().get().data, __rt_ctx->intern_keyword("dynamic").expect_ok())) };
            return util::format("{}->bind_root({})->with_meta({})->set_dynamic({})",
                                runtime::munge(var.native_name),
                                val.str(true),
                                runtime::munge(meta.unwrap().get().native_name),
                                dynamic);
          }
          else
          {
            return util::format("{}->bind_root({})->with_meta(jank::runtime::jank_nil)",
                                runtime::munge(var.native_name),
                                val.str(true));
          }
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return ");
        }
        [[fallthrough]];
      case analyze::expression_position::statement:
        {
          if(meta.is_some())
          {
            auto const dynamic{ truthy(
              get(meta.unwrap().get().data, __rt_ctx->intern_keyword("dynamic").expect_ok())) };
            util::format_to(body_buffer,
                            "{}->bind_root({})->with_meta({})->set_dynamic({});",
                            runtime::munge(var.native_name),
                            val.str(true),
                            runtime::munge(meta.unwrap().get().native_name),
                            dynamic);
          }
          else
          {
            util::format_to(body_buffer,
                            "{}->bind_root({})->with_meta(jank::runtime::jank_nil);",
                            runtime::munge(var.native_name),
                            val.str(true));
          }
          return none;
        }
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::var_deref_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool const)
  {
    auto const &var(expr->frame->find_lifted_var(expr->qualified_name).unwrap().get());
    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        {
          return util::format("{}->deref()", runtime::munge(var.native_name));
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return {}->deref();", runtime::munge(var.native_name));
          return none;
        }
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::var_ref_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool const)
  {
    auto const &var(expr->frame->find_lifted_var(expr->qualified_name).unwrap().get());
    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        {
          return runtime::munge(var.native_name);
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return {};", runtime::munge(var.native_name));
          return none;
        }
    }
  }

  void processor::format_elided_var(jtl::immutable_string const &start,
                                    jtl::immutable_string const &end,
                                    jtl::immutable_string const &ret_tmp,
                                    native_vector<analyze::expression_ref> const &arg_exprs,
                                    analyze::expr::function_arity const &fn_arity,
                                    bool const arg_box_needed,
                                    bool const ret_box_needed)
  {
    /* TODO: Assert arg count when we know it. */
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap());
    }

    jtl::immutable_string ret_box;
    if(ret_box_needed)
    {
      ret_box = "jank::runtime::make_box(";
    }
    util::format_to(body_buffer, "auto const {}({}{}", ret_tmp, ret_box, start);
    bool need_comma{};
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      if(need_comma)
      {
        util::format_to(body_buffer, ", ");
      }
      util::format_to(body_buffer, "{}", arg_tmps[i].str(arg_box_needed));
      need_comma = true;
    }
    util::format_to(body_buffer, "{}{});", end, (ret_box_needed ? ")" : ""));
  }

  void processor::format_direct_call(jtl::immutable_string const &source_tmp,
                                     jtl::immutable_string const &ret_tmp,
                                     native_vector<analyze::expression_ref> const &arg_exprs,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const arg_box_needed)
  {
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap());
    }

    util::format_to(body_buffer, "auto const {}({}.call(", ret_tmp, source_tmp);

    bool need_comma{};
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      if(need_comma)
      {
        util::format_to(body_buffer, ", ");
      }
      util::format_to(body_buffer, "{}", arg_tmps[i].str(true));
      need_comma = true;
    }
    util::format_to(body_buffer, "));");
  }

  void processor::format_dynamic_call(jtl::immutable_string const &source_tmp,
                                      jtl::immutable_string const &ret_tmp,
                                      native_vector<analyze::expression_ref> const &arg_exprs,
                                      analyze::expr::function_arity const &fn_arity,
                                      bool const arg_box_needed)
  {
    //util::println("format_dynamic_call source {}", source_tmp);
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      //util::println("\tformat_dynamic_call arg {}",
      //              runtime::to_code_string(arg_expr->to_runtime_data()));
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, arg_box_needed).unwrap());
    }

    util::format_to(body_buffer,
                    "auto const {}(jank::runtime::dynamic_call({}",
                    ret_tmp,
                    source_tmp);
    for(size_t i{}; i < runtime::max_params && i < arg_tmps.size(); ++i)
    {
      util::format_to(body_buffer, ", {}", arg_tmps[i].str(true));
    }

    if(runtime::max_params < arg_tmps.size())
    {
      util::format_to(
        body_buffer,
        ", jank::runtime::make_box<jank::runtime::obj::persistent_list>(std::in_place");
      for(size_t i{ runtime::max_params }; i < arg_tmps.size(); ++i)
      {
        util::format_to(body_buffer, ", {}", arg_tmps[i].str(true));
      }
      util::format_to(body_buffer, ")");
    }
    util::format_to(body_buffer, "));");
  }

  jtl::option<handle> processor::gen(analyze::expr::call_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const box_needed)
  {
    /* TODO: Doesn't take into account boxing. */
    handle ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("call")) };
    /* Clojure's codegen actually skips vars for certain calls to clojure.core
     * fns; this is not the same as direct linking, which uses `invokeStatic`
     * instead. Rather, this makes calls to `get` become `RT.get`, calls to `+` become
     * `Numbers.add`, and so on. We do the same thing here. */
    bool elided{};
    /* TODO: Use the actual var meta to do this, not a hard-coded set of if checks. */
    if(auto const * const ref = dynamic_cast<analyze::expr::var_deref *>(expr->source_expr.data))
    {
      auto const &name{ ref->var->name->name };
      if(ref->var->n->name->name != "clojure.core")
      {
      }
      else if(name == "get")
      {
        format_elided_var("jank::runtime::get(",
                          ")",
                          ret_tmp.str(false),
                          expr->arg_exprs,
                          fn_arity,
                          true,
                          false);
        elided = true;
      }
      else if(expr->arg_exprs.empty())
      {
        if(name == "rand")
        {
          format_elided_var("jank::runtime::rand(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
      }
      else if(expr->arg_exprs.size() == 1)
      {
        //if(name == "print")
        //{
        //  format_elided_var("jank::runtime::print(",
        //                    ")",
        //                    ret_tmp.str(false),
        //                    expr->arg_exprs,
        //                    fn_arity,
        //                    true,
        //                    false);
        //  elided = true;
        //}
        if(name == "abs")
        {
          format_elided_var("jank::runtime::abs(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "sqrt")
        {
          format_elided_var("jank::runtime::sqrt(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "int")
        {
          format_elided_var("jank::runtime::to_int(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "seq")
        {
          format_elided_var("jank::runtime::seq(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(name == "fresh-seq")
        {
          format_elided_var("jank::runtime::fresh_seq(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(name == "first")
        {
          format_elided_var("jank::runtime::first(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(name == "next")
        {
          format_elided_var("jank::runtime::next(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(name == "next-in-place")
        {
          format_elided_var("jank::runtime::next_in_place(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
        else if(name == "nil?")
        {
          format_elided_var("jank::runtime::is_nil(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            box_needed);
          elided = true;
        }
        else if(name == "some?")
        {
          format_elided_var("jank::runtime::is_some(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            box_needed);
          elided = true;
        }
      }
      else if(expr->arg_exprs.size() == 2)
      {
        if(name == "+")
        {
          format_elided_var("jank::runtime::add(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "-")
        {
          format_elided_var("jank::runtime::sub(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "*")
        {
          format_elided_var("jank::runtime::mul(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "/")
        {
          format_elided_var("jank::runtime::div(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "<")
        {
          format_elided_var("jank::runtime::lt(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "<=")
        {
          format_elided_var("jank::runtime::lte(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == ">")
        {
          format_elided_var("jank::runtime::lt(",
                            ")",
                            ret_tmp.str(false),
                            { expr->arg_exprs.rbegin(), expr->arg_exprs.rend() },
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == ">=")
        {
          format_elided_var("jank::runtime::lte(",
                            ")",
                            ret_tmp.str(false),
                            { expr->arg_exprs.rbegin(), expr->arg_exprs.rend() },
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "min")
        {
          format_elided_var("jank::runtime::min(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "max")
        {
          format_elided_var("jank::runtime::max(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "pow")
        {
          format_elided_var("jank::runtime::pow(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            false,
                            box_needed);
          elided = true;
          ret_tmp = { ret_tmp.unboxed_name, box_needed };
        }
        else if(name == "conj")
        {
          format_elided_var("jank::runtime::conj(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
      }
      else if(expr->arg_exprs.size() == 3)
      {
        if(name == "assoc")
        {
          format_elided_var("jank::runtime::assoc(",
                            ")",
                            ret_tmp.str(false),
                            expr->arg_exprs,
                            fn_arity,
                            true,
                            false);
          elided = true;
        }
      }
    }
    else if(auto const * const fn = dynamic_cast<analyze::expr::function *>(expr->source_expr.data))
    {
      bool variadic{};
      for(auto const &arity : fn->arities)
      {
        if(arity.fn_ctx->is_variadic)
        {
          variadic = true;
        }
      }
      if(!variadic)
      {
        auto const &source_tmp(gen(expr->source_expr, fn_arity, false));
        format_direct_call(source_tmp.unwrap().str(false),
                           ret_tmp.str(true),
                           expr->arg_exprs,
                           fn_arity,
                           true);
        elided = true;
      }
    }

    if(!elided)
    {
      auto const &source_tmp(gen(expr->source_expr, fn_arity, false));
      format_dynamic_call(source_tmp.unwrap().str(true),
                          ret_tmp.str(true),
                          expr->arg_exprs,
                          fn_arity,
                          true);
    }

    if(expr->position == analyze::expression_position::tail)
    {
      /* TODO: Box here, not in the calls above. Using false when we mean true is not good. */
      /* No need for extra boxing on this, since the boxing was done on the call above. */
      util::format_to(body_buffer, "return {};", ret_tmp.str(false));
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::primitive_literal_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool const)
  {
    auto const &constant(expr->frame->find_lifted_constant(expr->data).unwrap().get());

    handle ret{ runtime::munge(constant.native_name) };
    if(constant.unboxed_native_name.is_some())
    {
      ret = { runtime::munge(constant.native_name),
              runtime::munge(constant.unboxed_native_name.unwrap()) };
    }

    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        {
          return ret;
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return {};", ret.str(expr->needs_box));
          return none;
        }
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::vector_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    native_vector<handle> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr, fn_arity, true).unwrap());
    }

    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("vec")));
    util::format_to(body_buffer,
                    "auto const {}(jank::runtime::make_box<jank::runtime::obj::persistent_vector>(",
                    ret_tmp);
    if(expr->meta.is_some())
    {
      detail::gen_constant(expr->meta.unwrap(), body_buffer, true);
      util::format_to(body_buffer, ", ");
    }
    util::format_to(body_buffer, "std::in_place ");
    for(auto const &tmp : data_tmps)
    {
      util::format_to(body_buffer, ", ");
      util::format_to(body_buffer, "{}", tmp.str(true));
    }
    util::format_to(body_buffer, "));");

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::map_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    native_vector<std::pair<handle, handle>> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr.first, fn_arity, true).unwrap(),
                             gen(data_expr.second, fn_arity, true).unwrap());
    }

    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("map")));

    /* Jump right to a hash map, if we have enough values. */
    if(expr->data_exprs.size() <= runtime::obj::persistent_array_map::max_size)
    {
      util::format_to(body_buffer, "auto const {}(", ret_tmp);
      bool need_comma{};
      if(expr->meta.is_some())
      {
        util::format_to(body_buffer,
                        "jank::runtime::obj::persistent_array_map::create_unique_with_meta(");
        detail::gen_constant(expr->meta.unwrap(), body_buffer, true);
        need_comma = true;
      }
      else
      {
        util::format_to(body_buffer, "jank::runtime::obj::persistent_array_map::create_unique(");
      }
      for(auto const &data_tmp : data_tmps)
      {
        if(need_comma)
        {
          util::format_to(body_buffer, ", ");
        }
        util::format_to(body_buffer, "{}", data_tmp.first.str(true));
        util::format_to(body_buffer, ", {}", data_tmp.second.str(true));
        need_comma = true;
      }
      util::format_to(body_buffer, "));");
    }
    else
    {
      bool need_comma{};
      util::format_to(body_buffer, "auto const {}(", ret_tmp);
      if(expr->meta.is_some())
      {
        util::format_to(body_buffer,
                        "jank::runtime::obj::persistent_hash_map::create_unique_with_meta(");
        detail::gen_constant(expr->meta.unwrap(), body_buffer, true);
        need_comma = true;
      }
      else
      {
        util::format_to(body_buffer, "jank::runtime::obj::persistent_hash_map::create_unique(");
      }
      for(auto const &data_tmp : data_tmps)
      {
        if(need_comma)
        {
          util::format_to(body_buffer, ", ");
        }
        util::format_to(body_buffer, "std::make_pair(");
        util::format_to(body_buffer, "{}", data_tmp.first.str(true));
        util::format_to(body_buffer, ", {})", data_tmp.second.str(true));
        need_comma = true;
      }
      util::format_to(body_buffer, "));");
    }

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::set_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    native_vector<handle> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr, fn_arity, true).unwrap());
    }

    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("set")));
    util::format_to(
      body_buffer,
      "auto const {}(jank::runtime::make_box<jank::runtime::obj::persistent_hash_set>(",
      ret_tmp);
    if(expr->meta.is_some())
    {
      detail::gen_constant(expr->meta.unwrap(), body_buffer, true);
      util::format_to(body_buffer, ", ");
    }
    util::format_to(body_buffer, "std::in_place ");
    for(auto const &tmp : data_tmps)
    {
      util::format_to(body_buffer, ", ");
      util::format_to(body_buffer, "{}", tmp.str(true));
    }
    util::format_to(body_buffer, "));");

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::local_reference_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool const)
  {
    auto const munged_name(runtime::munge(expr->binding->native_name));

    handle ret;
    if(expr->binding->needs_box)
    {
      ret = munged_name;
    }
    else
    {
      ret = handle{ detail::boxed_local_name(munged_name), munged_name };
    }

    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        {
          return ret;
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return {};", ret.str(expr->needs_box));
          return none;
        }
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::function_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool const box_needed)
  {
    auto const compiling(truthy(__rt_ctx->compile_files_var->deref()));
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ expr,
                   module,
                   //runtime::module::nest_module(module, runtime::munge(expr->unique_name)),
                   compiling ? compilation_target::function : compilation_target::eval };

    /* If we're compiling, we'll create a separate file for this. */
    //if(target != compilation_target::module)
    {
      util::format_to(deps_buffer, "{}", prc.declaration_str());
    }

    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        /* TODO: Return a handle. */
        {
          return prc.expression_str(box_needed);
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return {};", prc.expression_str(box_needed));
          return none;
        }
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::recur_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity, true).unwrap());
    }

    auto arg_tmp_it(arg_tmps.begin());
    for(auto const &param : fn_arity.params)
    {
      util::format_to(body_buffer, "{} = {};", runtime::munge(param->name), arg_tmp_it->str(true));
      ++arg_tmp_it;
    }
    util::format_to(body_buffer, "continue;");
    return none;
  }

  /* NOLINTNEXTLINE(readability-make-member-function-const): Can't be const, due to overload resolution. */
  jtl::option<handle> processor::gen(analyze::expr::recursion_reference_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool const)
  {
    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", munge(expr->fn_ctx->fn->name));
      return none;
    }
    return munge(expr->fn_ctx->fn->name);
  }

  jtl::option<handle> processor::gen(analyze::expr::named_recursion_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    handle ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("named_recursion")) };

    auto const &source_tmp(
      gen(jtl::ref<analyze::expr::recursion_reference>{ &expr->recursion_ref }, fn_arity, false));
    format_dynamic_call(source_tmp.unwrap().str(true),
                        ret_tmp.str(true),
                        expr->arg_exprs,
                        fn_arity,
                        true);

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp.str(true));
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::let_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    handle const ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("let")),
                          expr->needs_box };
    bool used_option{};

    if(expr->needs_box)
    {
      /* TODO: The type may not be default constructible so this may fail. We likely
       * want an array the same size as the desired type. When we have the last expression,
       * we can then do a placement new with the move ctor.
       *
       * Also add a test for this. */
      auto const last_expr_type{ cpp_util::expression_type(
        expr->body->values[expr->body->values.size() - 1]) };

      jtl::immutable_string type_name;
      /* In analysis, we treat untyped objects as object*, since that's easier for IR.
       * However, for C++, we want to normalize that to object_ref to take full advantage
       * of richer types. */
      if(cpp_util::is_untyped_object(last_expr_type))
      {
        type_name = "object_ref";
        util::format_to(body_buffer, "{} {}{ }; {", type_name, ret_tmp.str(expr->needs_box));
      }
      else
      {
        used_option = true;
        type_name = Cpp::GetTypeAsString(Cpp::GetNonReferenceType(last_expr_type));
        /* TODO: Test for this with something non-default constructible. */
        util::format_to(body_buffer,
                        "jtl::option<{}> {}{ }; {",
                        type_name,
                        ret_tmp.str(expr->needs_box));
      }
    }
    else
    {
      util::format_to(body_buffer,
                      "auto const {}([&](){}{",
                      ret_tmp.str(expr->needs_box),
                      (expr->needs_box ? "-> object_ref" : ""));
    }

    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      if(local.is_none())
      {
        throw std::runtime_error{ util::format("ICE: unable to find local: {}",
                                               pair.first->to_string()) };
      }

      auto const &val_tmp(gen(pair.second, fn_arity, pair.second->needs_box));
      auto const &munged_name(runtime::munge(local.unwrap().binding->native_name));
      /* Every binding is wrapped in its own scope, to allow shadowing.
       *
       * Also, bindings are references to their value expression, rather than a copy.
       * This is important for C++ interop, since the we don't want to, and we may not
       * be able to, just copy stack-allocated C++ objects around willy nillly. */
      util::format_to(body_buffer, "{ auto &&{}({}); ", munged_name, val_tmp.unwrap().str(false));

      auto const binding(local.unwrap().binding);
      if(!binding->needs_box && binding->has_boxed_usage)
      {
        util::format_to(body_buffer,
                        "auto const {}({});",
                        detail::boxed_local_name(munged_name),
                        val_tmp.unwrap().str(true));
      }
    }

    for(auto it(expr->body->values.begin()); it != expr->body->values.end();)
    {
      auto const &val_tmp(gen(*it, fn_arity, true));

      /* We ignore all values but the last. */
      if(++it == expr->body->values.end() && val_tmp.is_some())
      {
        if(expr->needs_box)
        {
          /* The last expression tmp needs to be movable. */
          util::format_to(body_buffer,
                          "{} = std::move({});",
                          ret_tmp.str(true),
                          val_tmp.unwrap().str(expr->needs_box));
        }
        else
        {
          util::format_to(body_buffer, "return {};", val_tmp.unwrap().str(expr->needs_box));
        }
      }
    }
    for(auto const &_ : expr->pairs)
    {
      static_cast<void>(_);
      util::format_to(body_buffer, "}");
    }

    if(expr->needs_box)
    {
      util::format_to(body_buffer, "}");
    }
    else
    {
      util::format_to(body_buffer, "}());");
    }

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer,
                      "return {}{};",
                      ret_tmp.str(expr->needs_box),
                      (used_option ? ".unwrap()" : ""));
      return none;
    }

    return util::format("{}{}", ret_tmp.str(expr->needs_box), (used_option ? ".unwrap()" : ""));
  }

  jtl::option<handle>
  processor::gen(analyze::expr::letfn_ref const, analyze::expr::function_arity const &, bool const)
  {
    return none;
  }

  jtl::option<handle> processor::gen(analyze::expr::do_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool const)
  {
    jtl::option<handle> last;
    for(auto const &form : expr->values)
    {
      last = gen(form, arity, true);
    }

    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        {
          return last;
        }
      case analyze::expression_position::tail:
        {
          if(last.is_none())
          {
            util::format_to(body_buffer, "return jank::runtime::jank_nil;");
          }
          else
          {
            util::format_to(body_buffer, "return {};", last.unwrap().str(expr->needs_box));
          }
          return none;
        }
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::if_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    /* TODO: Handle unboxed results! */
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("if")));
    util::format_to(body_buffer, "object_ref {}{ };", ret_tmp);
    auto const &condition_tmp(gen(expr->condition, fn_arity, false));
    util::format_to(body_buffer,
                    "if(jank::runtime::truthy({})) {",
                    condition_tmp.unwrap().str(false));
    auto const &then_tmp(gen(expr->then, fn_arity, true));
    if(then_tmp.is_some())
    {
      util::format_to(body_buffer, "{} = {}; }", ret_tmp, then_tmp.unwrap().str(expr->needs_box));
    }
    else
    {
      util::format_to(body_buffer, "}");
    }

    if(expr->else_.is_some())
    {
      util::format_to(body_buffer, "else {");
      auto const &else_tmp(gen(expr->else_.unwrap(), fn_arity, true));
      if(else_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {}; }", ret_tmp, else_tmp.unwrap().str(expr->needs_box));
      }
      else
      {
        util::format_to(body_buffer, "}");
      }
    }
    /* If we don't have an else, but we're in return position, we need to be sure to return
     * something, so we return nil. */
    else if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "else { return {}; }", ret_tmp);
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::throw_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const)
  {
    auto const &value_tmp(gen(expr->value, fn_arity, true));
    /* We static_cast to object_ref here, since we'll be trying to catch an object_ref in any
     * try/catch forms. This loses us our type info, but C++ doesn't do implicit conversions
     * when catching and we're not using inheritance. */
    util::format_to(body_buffer,
                    "throw static_cast<jank::runtime::object_ref>({});",
                    value_tmp.unwrap().str(true));
    return none;
  }

  jtl::option<handle> processor::gen(analyze::expr::try_ref const expr,
                                     analyze::expr::function_arity const &fn_arity,
                                     bool const box_needed)
  {
    auto const has_catch{ !expr->catch_bodies.empty() };
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("try")));
    util::format_to(body_buffer, "object_ref {}{ };", ret_tmp);

    util::format_to(body_buffer, "{");
    if(expr->finally_body.is_some())
    {
      util::format_to(body_buffer, "jank::util::scope_exit const finally{ [&](){ ");
      gen(expr->finally_body.unwrap(), fn_arity, box_needed);
      util::format_to(body_buffer, "} };");
    }

    if(has_catch)
    {
      util::format_to(body_buffer, "try {");
      auto const &body_tmp(gen(expr->body, fn_arity, box_needed));
      if(body_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, body_tmp.unwrap().str(box_needed));
      }
      if(expr->position == analyze::expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
      }
      util::format_to(body_buffer, "}");

      /* There's a gotcha here, tied to how we throw exceptions. We're catching an object_ref, which
     * means we need to be throwing an object_ref. Since we're not using inheritance, we can't
     * rely on a catch-all and C++ doesn't do implicit conversions into catch types. So, if we
     * throw a persistent_string_ref, for example, it will not be caught as an object_ref.
     *
     * We mitigate this by ensuring during the codegen for throw that we type-erase to
     * an object_ref.
     */
      util::format_to(body_buffer,
                      "catch(jank::runtime::object_ref const {}) {",
                      runtime::munge(expr->catch_bodies[0].sym->name));
      auto const &catch_tmp(gen(expr->catch_bodies[0].body, fn_arity, box_needed));
      if(catch_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, catch_tmp.unwrap().str(box_needed));
      }
      if(expr->position == analyze::expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
      }
      util::format_to(body_buffer, "}");
    }
    else
    {
      auto const &body_tmp(gen(expr->body, fn_arity, box_needed));
      if(body_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, body_tmp.unwrap().str(box_needed));
      }
      if(expr->position == analyze::expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
      }
    }

    util::format_to(body_buffer, "}");

    return ret_tmp;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::case_ref const, analyze::expr::function_arity const &, bool)
  {
    return none;
  }

  jtl::option<handle>
  processor::gen(expr::cpp_raw_ref const expr, expr::function_arity const &, bool)
  {
    util::format_to(deps_buffer, "{}", expr->code);

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return jank::runtime::jank_nil;");
      return none;
    }
    return none;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_type_ref const, analyze::expr::function_arity const &, bool)
  {
    throw std::runtime_error{ "cpp_type has no codegen" };
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_value_ref const expr,
                                     analyze::expr::function_arity const &,
                                     bool)
  {
    if(expr->val_kind == expr::cpp_value::value_kind::null)
    {
      if(expr->position == expression_position::tail)
      {
        util::format_to(body_buffer, "return nullptr;");
        return none;
      }
      return "nullptr";
    }
    if(expr->val_kind == expr::cpp_value::value_kind::bool_true
       || expr->val_kind == expr::cpp_value::value_kind::bool_false)
    {
      auto const val{ expr->val_kind == expr::cpp_value::value_kind::bool_true };
      if(expr->position == expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", val);
        return none;
      }
      return util::format("{}", val);
    }

    auto tmp{ Cpp::GetQualifiedCompleteName(expr->scope) };

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", tmp);
      return none;
    }

    return tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_cast_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool const box_needed)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_cast")));
    auto const value_tmp{ gen(expr->value_expr, arity, box_needed) };

    util::format_to(
      body_buffer,
      "auto const {}{ jank::runtime::convert<{}>::{}({}) };",
      ret_tmp,
      Cpp::GetTypeAsString(expr->conversion_type),
      (expr->policy == conversion_policy::into_object ? "into_object" : "from_object"),
      value_tmp.unwrap().str(true));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_call_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool const)
  {
    if(expr->source_expr->kind == expression_kind::cpp_value)
    {
      auto const source{ static_cast<expr::cpp_value *>(expr->source_expr.data) };
      auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_call")));

      native_vector<handle> arg_tmps;
      arg_tmps.reserve(expr->arg_exprs.size());
      for(auto const &arg_expr : expr->arg_exprs)
      {
        arg_tmps.emplace_back(gen(arg_expr, arity, false).unwrap());
      }

      util::format_to(body_buffer,
                      "auto const {}{ {}(",
                      ret_tmp,
                      Cpp::GetQualifiedCompleteName(source->scope));

      bool need_comma{};
      for(auto const &arg_tmp : arg_tmps)
      {
        if(need_comma)
        {
          util::format_to(body_buffer, ", ");
        }
        util::format_to(body_buffer, "{}", arg_tmp.str(false));
        need_comma = true;
      }

      util::format_to(body_buffer, ") };");

      if(expr->position == expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
        return none;
      }

      return ret_tmp;
    }
    else
    {
      jank_debug_assert(false);
      return none;
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_constructor_call_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool const)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_ctor")));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, arity, false).unwrap());
    }

    if(expr->arg_exprs.empty())
    {
      util::format_to(body_buffer, "{} {}{ };", Cpp::GetTypeAsString(expr->type), ret_tmp);
      return ret_tmp;
    }

    util::format_to(body_buffer, "{} {}( ", Cpp::GetTypeAsString(expr->type), ret_tmp);

    bool need_comma{};
    for(auto const &arg_tmp : arg_tmps)
    {
      if(need_comma)
      {
        util::format_to(body_buffer, ", ");
      }
      util::format_to(body_buffer, "{}", arg_tmp.str(false));
      need_comma = true;
    }

    util::format_to(body_buffer, " );");

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }
    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_member_call_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool)
  {
    auto const fn_name{ Cpp::GetName(expr->fn) };
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string(fn_name)));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, arity, false).unwrap());
    }

    util::format_to(
      body_buffer,
      "auto &&{}{ {}{}{}(",
      ret_tmp,
      arg_tmps[0].str(false),
      (Cpp::IsPointerType(cpp_util::expression_type(expr->arg_exprs[0])) ? "->" : "."),
      fn_name);

    bool need_comma{};
    for(auto it{ arg_tmps.begin() + 1 }; it != arg_tmps.end(); ++it)
    {
      if(need_comma)
      {
        util::format_to(body_buffer, ", ");
      }
      util::format_to(body_buffer, "{}", it->str(false));
      need_comma = true;
    }

    util::format_to(body_buffer, ") };");

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_member_access_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string(expr->name)));
    auto obj_tmp(gen(expr->obj_expr, arity, false));

    util::format_to(body_buffer,
                    "auto &&{}{ {}{}{} };",
                    ret_tmp,
                    obj_tmp.unwrap().str(false),
                    (Cpp::IsPointerType(cpp_util::expression_type(expr->obj_expr)) ? "->" : "."),
                    expr->name);

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_builtin_operator_call_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_operator")));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, arity, false).unwrap());
    }

    if(expr->arg_exprs.size() == 1)
    {
      util::format_to(body_buffer,
                      "auto {}( {}{} );",
                      ret_tmp,
                      cpp_util::operator_name(static_cast<Cpp::Operator>(expr->op)).unwrap(),
                      arg_tmps[0].str(false));
    }
    else
    {
      util::format_to(body_buffer,
                      "auto {}( {} {} {} );",
                      ret_tmp,
                      arg_tmps[0].str(false),
                      cpp_util::operator_name(static_cast<Cpp::Operator>(expr->op)).unwrap(),
                      arg_tmps[1].str(false));
    }

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_box_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool)
  {
    auto ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("cpp_box")) };
    auto value_tmp{ gen(expr->value_expr, arity, false) };

    util::format_to(body_buffer,
                    "auto {}{ jank::runtime::make_box<jank::runtime::obj::opaque_box>({}) };",
                    ret_tmp,
                    value_tmp.unwrap().str(false));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_unbox_ref const expr,
                                     analyze::expr::function_arity const &arity,
                                     bool)
  {
    auto ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("cpp_unbox")) };
    auto value_tmp{ gen(expr->value_expr, arity, false) };

    util::format_to(body_buffer,
                    "auto {}{ "
                    "static_cast<{}>(jank::runtime::try_object<jank::runtime::obj::opaque_box>({})-"
                    ">data.data) };",
                    ret_tmp,
                    Cpp::GetTypeAsString(expr->type),
                    value_tmp.unwrap().str(false));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::immutable_string processor::declaration_str()
  {
    if(!generated_declaration)
    {
      build_header();
      build_body();
      build_footer();
      generated_declaration = true;
    }

    native_transient_string ret;
    ret.reserve(deps_buffer.size() + header_buffer.size() + body_buffer.size()
                + footer_buffer.size());
    ret += jtl::immutable_string_view{ deps_buffer.data(), deps_buffer.size() };
    ret += jtl::immutable_string_view{ header_buffer.data(), header_buffer.size() };
    ret += jtl::immutable_string_view{ body_buffer.data(), body_buffer.size() };
    ret += jtl::immutable_string_view{ footer_buffer.data(), footer_buffer.size() };

    //ret = util::format_cpp_source(ret).expect_ok();

    //util::println("codegen declaration {}", ret);
    return ret;
  }

  void processor::build_header()
  {
    /* TODO: We don't want this for nested modules, but we do if they're in their own file.
     * Do we need three module compilation targets? Top-level, nested, local?
     *
     * Local fns are within a struct already, so we can't enter the ns again. */
    //if(!runtime::module::is_nested_module(module))
    //if(target == compilation_target::module)
    {
      util::format_to(header_buffer,
                      "namespace {} {",
                      runtime::module::module_to_native_ns(module));
    }

    util::format_to(header_buffer,
                    R"(
        struct {} : jank::runtime::obj::jit_function
        {
      )",
                    runtime::munge(struct_name.name));

    {
      /* TODO: Constants and vars are not shared across arities. We'd need stable names. */
      native_set<i64> used_vars, used_constants, used_captures;
      for(auto const &arity : root_fn->arities)
      {
        for(auto const &v : arity.frame->lifted_vars)
        {
          if(used_vars.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_vars.emplace(v.second.native_name.to_hash());

          util::format_to(header_buffer,
                          "jank::runtime::var_ref const {};",
                          runtime::munge(v.second.native_name));
        }

        for(auto const &v : arity.frame->lifted_constants)
        {
          if(used_constants.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_constants.emplace(v.second.native_name.to_hash());

          util::format_to(header_buffer,
                          "{} const {};",
                          detail::gen_constant_type(v.second.data, true),
                          runtime::munge(v.second.native_name));

          if(v.second.unboxed_native_name.is_some())
          {
            util::format_to(header_buffer,
                            "static constexpr {} const {}{ ",
                            detail::gen_constant_type(v.second.data, false),
                            runtime::munge(v.second.unboxed_native_name.unwrap()));
            detail::gen_constant(v.second.data, header_buffer, false);
            util::format_to(header_buffer, "};");
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

          util::format_to(header_buffer,
                          "jank::runtime::object_ref const {};",
                          runtime::munge(v.second.native_name));
        }
      }
    }

    {
      native_set<i64> used_captures;
      util::format_to(header_buffer, "{}(", runtime::munge(struct_name.name));

      bool need_comma{};
      for(auto const &arity : root_fn->arities)
      {
        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          /* TODO: More useful types here. */
          util::format_to(header_buffer,
                          "{} jank::runtime::object_ref {}",
                          (need_comma ? "," : ""),
                          runtime::munge(v.second.native_name));
          need_comma = true;
        }
      }
    }

    {
      native_set<i64> used_vars, used_constants, used_captures;
      util::format_to(header_buffer, ") : jank::runtime::obj::jit_function{ ");
      /* TODO: All of the meta in clojure.core alone costs 2s to JIT compile at run-time.
       * How can this be faster? */
      detail::gen_constant(root_fn->meta, header_buffer, true);
      util::format_to(header_buffer, "}");

      for(auto const &arity : root_fn->arities)
      {
        for(auto const &v : arity.frame->lifted_vars)
        {
          if(used_vars.contains(v.second.native_name.to_hash()))
          {
            continue;
          }
          used_vars.emplace(v.second.native_name.to_hash());

          util::format_to(header_buffer,
                          R"(, {}{ jank::runtime::__rt_ctx->intern_var("{}", "{}").expect_ok() })",
                          runtime::munge(v.second.native_name),
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

          util::format_to(header_buffer, ", {}{", runtime::munge(v.second.native_name));
          detail::gen_constant(v.second.data, header_buffer, true);
          util::format_to(header_buffer, "}");
        }

        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          auto const name{ runtime::munge(v.second.native_name) };
          util::format_to(header_buffer, ", {}{ {} }", name, name);
        }
      }
    }

    util::format_to(header_buffer, "{  }");
  }

  void processor::build_body()
  {
    analyze::expr::function_arity const *variadic_arity{};
    analyze::expr::function_arity const *highest_fixed_arity{};
    for(auto const &arity : root_fn->arities)
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

      jtl::immutable_string recur_suffix;
      if(arity.fn_ctx->is_tail_recursive)
      {
        recur_suffix = detail::recur_suffix;
      }

      util::format_to(body_buffer, "jank::runtime::object_ref call(");
      bool param_comma{};
      bool param_shadows_fn{};
      for(auto const &param : arity.params)
      {
        util::format_to(body_buffer,
                        "{} jank::runtime::object_ref const {}{}",
                        (param_comma ? ", " : ""),
                        runtime::munge(param->name),
                        recur_suffix);
        param_comma = true;
        param_shadows_fn |= param->name == root_fn->name;
      }

      util::format_to(body_buffer,
                      R"(
          ) final {
          using namespace jank;
          using namespace jank::runtime;
        )");

      //util::format_to(body_buffer, "jank::profile::timer __timer{ \"{}\" };", root_fn->name);

      if(!param_shadows_fn)
      {
        util::format_to(body_buffer, "object_ref const {}{ this };", runtime::munge(root_fn->name));
      }

      if(arity.fn_ctx->is_tail_recursive)
      {
        util::format_to(body_buffer, "{");

        for(auto const &param : arity.params)
        {
          auto const name{ runtime::munge(param->name) };
          util::format_to(body_buffer, "auto {}({}{});", name, name, recur_suffix);
        }

        util::format_to(body_buffer,
                        R"(
            while(true)
            {
          )");
      }

      for(auto const &form : arity.body->values)
      {
        gen(form, arity, true);
      }

      if(arity.body->values.empty())
      {
        util::format_to(body_buffer, "return jank::runtime::jank_nil;");
      }

      if(arity.fn_ctx->is_tail_recursive)
      {
        util::format_to(body_buffer, "} }");
      }

      util::format_to(body_buffer, "}");
    }

    if(variadic_arity)
    {
      bool const variadic_ambiguous{ highest_fixed_arity
                                     && highest_fixed_arity->fn_ctx->param_count
                                       == variadic_arity->fn_ctx->param_count - 1 };

      util::format_to(body_buffer,
                      R"(
          jank::runtime::behavior::callable::arity_flag_t get_arity_flags() const final
          { return jank::runtime::behavior::callable::build_arity_flags({}, true, {}); }
        )",
                      variadic_arity->fn_ctx->param_count - 1,
                      variadic_ambiguous);
    }
  }

  void processor::build_footer()
  {
    /* Struct. */
    util::format_to(footer_buffer, "};");

    /* Namespace. */
    //if(!runtime::module::is_nested_module(module))
    //if(target == compilation_target::module)
    {
      util::format_to(footer_buffer, "}");
    }

    if(target == compilation_target::module)
    {
      util::format_to(footer_buffer,
                      "extern \"C\" void* {}(){",
                      runtime::module::module_to_load_function(module));
      util::format_to(footer_buffer,
                      "return {}::{}{ }.call().erase();",
                      runtime::module::module_to_native_ns(module),
                      runtime::munge(struct_name.name));
      util::format_to(footer_buffer, "}");
    }
  }

  jtl::immutable_string processor::expression_str(bool const box_needed)
  {
    auto const module_ns(runtime::module::module_to_native_ns(module));

    if(!generated_expression)
    {
      jtl::immutable_string close = ")";
      if(box_needed)
      {
        util::format_to(
          expression_buffer,
          "jank::runtime::make_box<{}>(",
          runtime::module::nest_native_ns(module_ns, runtime::munge(struct_name.name)));
      }
      else
      {
        util::format_to(
          expression_buffer,
          "{}{ ",
          runtime::module::nest_native_ns(module_ns, runtime::munge(struct_name.name)));
        close = "}";
      }

      native_set<uhash> used_captures;
      bool need_comma{};
      for(auto const &arity : root_fn->arities)
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
           * of it, which we're capturing. We need to reach further for that.
           *
           * We check for named recursion first, since that takes higher precedence
           * than locals or captures. */
          auto const recursion(root_fn->frame->find_named_recursion(v.first));
          if(recursion.is_some())
          {
            util::format_to(expression_buffer,
                            "{} {}",
                            (need_comma ? "," : ""),
                            munge(recursion.unwrap().fn_frame->fn_ctx->name));
          }
          else
          {
            auto const originating_local(root_fn->frame->find_local_or_capture(v.first));
            handle const h{ originating_local.unwrap().binding };
            util::format_to(expression_buffer,
                            "{} {}",
                            (need_comma ? "," : ""),
                            h.str(true),
                            originating_local.unwrap().binding->name->to_code_string(),
                            originating_local.unwrap().binding->native_name);
          }
          need_comma = true;
        }
      }

      util::format_to(expression_buffer, "{}", close);

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }

  /* TODO: Not sure if we want any of this. The module dependency loading feels wrong,
   * since it should be tied to calls to require instead. */
  jtl::immutable_string processor::module_init_str(jtl::immutable_string const &module_name)
  {
    jtl::string_builder module_buffer;

    util::format_to(module_buffer,
                    "namespace {} {",
                    runtime::module::module_to_native_ns(module_name));

    util::format_to(module_buffer,
                    R"(
        struct __ns__init
        {
      )");

    util::format_to(module_buffer, "static void __init(){");
    //util::format_to(module_buffer, "jank::profile::timer __timer{ \"ns __init\" };");
    util::format_to(module_buffer,
                    "constexpr auto const deps(jank::util::make_array<jtl::immutable_string>(");
    bool needs_comma{};
    for(auto const &dep : __rt_ctx->module_dependencies[module_name])
    {
      if(needs_comma)
      {
        util::format_to(module_buffer, ", ");
      }
      util::format_to(module_buffer, "\"/{}\"", dep);
      needs_comma = true;
    }
    util::format_to(module_buffer, "));");

    util::format_to(module_buffer, "for(auto const &dep : deps){");
    util::format_to(module_buffer, "jank::runtime::__rt_ctx->load_module(dep).expect_ok();");
    util::format_to(module_buffer, "}");

    /* __init fn */
    util::format_to(module_buffer, "}");

    /* Struct */
    util::format_to(module_buffer, "};");

    /* Namespace */
    util::format_to(module_buffer, "}");

    native_transient_string ret;
    ret.reserve(module_buffer.size());
    ret += jtl::immutable_string_view{ module_buffer.data(), module_buffer.size() };
    return ret;
  }
}
