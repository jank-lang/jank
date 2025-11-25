#include <Interpreter/Compatibility.h>
#include <clang/Interpreter/CppInterOp.h>

#include <jank/codegen/processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/meta.hpp>
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
 * object_ref thing_tmp(thing->call());
 * object_ref if_tmp;
 * if(foo)
 * { if_tmp = bar; }
 * else
 * { if_tmp = spam; }
 * println->call(thing_tmp, if_tmp);
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
          else if constexpr(std::same_as<T, runtime::obj::big_integer>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::big_integer>(\"{}\")",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::big_decimal>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::big_decimal>(\"{}\")",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::ratio>)
          {
            util::format_to(buffer,
                            "jank::runtime::obj::ratio::create({}, {})",
                            typed_o->data.numerator,
                            typed_o->data.denominator);
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
                            R"(jank::runtime::make_box<jank::runtime::obj::character>("{}"))",
                            util::escape(typed_o->to_string()));
          }
          else if constexpr(std::same_as<T, runtime::obj::keyword>)
          {
            util::format_to(
              buffer,
              R"(jank::runtime::__rt_ctx->intern_keyword("{}", "{}", true).expect_ok())",
              typed_o->sym->ns,
              typed_o->sym->name);
          }
          else if constexpr(std::same_as<T, runtime::obj::re_pattern>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::re_pattern>({}))",
                            /* We remove the # prefix here. */
                            typed_o->to_code_string().substr(1));
          }
          else if constexpr(std::same_as<T, runtime::obj::uuid>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::uuid>("{}"))",
                            typed_o->to_string());
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
            for(auto const it : runtime::make_sequence_range(typed_o))
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
      return local_name; // + "__boxed";
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

  jtl::immutable_string handle::str([[maybe_unused]] bool const needs_box) const
  {
    return boxed_name;
    //if(needs_box)
    //{
    //  if(boxed_name.empty())
    //  {
    //    throw std::runtime_error{ util::format("Missing boxed name for handle {}", unboxed_name) };
    //  }
    //  return boxed_name;
    //}
    //else
    //{
    //  return unboxed_name;
    //}
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

  jtl::option<handle>
  processor::gen(analyze::expression_ref const ex, analyze::expr::function_arity const &fn_arity)
  {
    jtl::option<handle> ret;
    visit_expr([&, this](auto const typed_ex) { ret = gen(typed_ex, fn_arity); }, ex);
    return ret;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::def_ref const expr, analyze::expr::function_arity const &fn_arity)
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

    auto const val(gen(expr->value.unwrap(), fn_arity).unwrap());
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

  jtl::option<handle>
  processor::gen(analyze::expr::var_deref_ref const expr, analyze::expr::function_arity const &)
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

  jtl::option<handle>
  processor::gen(analyze::expr::var_ref_ref const expr, analyze::expr::function_arity const &)
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

  void processor::format_dynamic_call(jtl::immutable_string const &source_tmp,
                                      jtl::immutable_string const &ret_tmp,
                                      native_vector<analyze::expression_ref> const &arg_exprs,
                                      analyze::expr::function_arity const &fn_arity)
  {
    //util::println("format_dynamic_call source {}", source_tmp);
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(arg_exprs.size());
    for(auto const &arg_expr : arg_exprs)
    {
      //util::println("\tformat_dynamic_call arg {}",
      //              runtime::to_code_string(arg_expr->to_runtime_data()));
      arg_tmps.emplace_back(gen(arg_expr, fn_arity).unwrap());
    }

    util::format_to(body_buffer,
                    "auto const {}(jank::runtime::dynamic_call({}",
                    ret_tmp,
                    source_tmp);
    for(size_t i{}; i < arg_tmps.size(); ++i)
    {
      util::format_to(body_buffer, ", {}", arg_tmps[i].str(true));
    }

    util::format_to(body_buffer, "));");
  }

  jtl::option<handle>
  processor::gen(analyze::expr::call_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    handle ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("call")) };
    auto const &source_tmp(gen(expr->source_expr, fn_arity));
    format_dynamic_call(source_tmp.unwrap().str(true),
                        ret_tmp.str(true),
                        expr->arg_exprs,
                        fn_arity);

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp.str(false));
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::primitive_literal_ref const expr,
                                     analyze::expr::function_arity const &)
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

  jtl::option<handle>
  processor::gen(analyze::expr::list_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    native_vector<handle> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr, fn_arity).unwrap());
    }

    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("list")));
    util::format_to(body_buffer,
                    "auto const {}(jank::runtime::make_box<jank::runtime::obj::persistent_list>(",
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

  jtl::option<handle> processor::gen(analyze::expr::vector_ref const expr,
                                     analyze::expr::function_arity const &fn_arity)
  {
    native_vector<handle> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr, fn_arity).unwrap());
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

  jtl::option<handle>
  processor::gen(analyze::expr::map_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    native_vector<std::pair<handle, handle>> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr.first, fn_arity).unwrap(),
                             gen(data_expr.second, fn_arity).unwrap());
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

  jtl::option<handle>
  processor::gen(analyze::expr::set_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    native_vector<handle> data_tmps;
    data_tmps.reserve(expr->data_exprs.size());
    for(auto const &data_expr : expr->data_exprs)
    {
      data_tmps.emplace_back(gen(data_expr, fn_arity).unwrap());
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
                                     analyze::expr::function_arity const &)
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

  jtl::option<handle>
  processor::gen(analyze::expr::function_ref const expr, analyze::expr::function_arity const &)
  {
    auto const compiling(truthy(__rt_ctx->compile_files_var->deref()));
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ expr,
                   module,
                   compiling ? compilation_target::function : compilation_target::eval };

    util::format_to(deps_buffer, "{}", prc.declaration_str());

    switch(expr->position)
    {
      case analyze::expression_position::statement:
      case analyze::expression_position::value:
        /* TODO: Return a handle. */
        {
          return prc.expression_str();
        }
      case analyze::expression_position::tail:
        {
          util::format_to(body_buffer, "return {};", prc.expression_str());
          return none;
        }
    }
  }

  jtl::option<handle>
  processor::gen(analyze::expr::recur_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, fn_arity).unwrap());
    }

    auto arg_tmp_it(arg_tmps.begin());
    if(expr->loop_target.is_some())
    {
      auto const let{ expr->loop_target.unwrap() };
      for(usize i{}; i < expr->arg_exprs.size(); ++i)
      {
        auto const &pair{ let->pairs[i] };
        auto const local(expr->frame->find_local_or_capture(pair.first));
        auto const &local_name(runtime::munge(local.unwrap().binding->native_name));
        auto const &val_name(arg_tmp_it->str(true));

        if(local_name != val_name)
        {
          util::format_to(body_buffer, "{} = {};", local_name, val_name);
        }
        ++arg_tmp_it;
      }

      util::format_to(body_buffer, "continue;");
    }
    else
    {
      for(auto const &param : fn_arity.params)
      {
        util::format_to(body_buffer,
                        "{} = {};",
                        runtime::munge(param->name),
                        arg_tmp_it->str(true));
        ++arg_tmp_it;
      }
      util::format_to(body_buffer, "continue;");
    }

    return none;
  }

  /* NOLINTNEXTLINE(readability-make-member-function-const): Can't be const, due to overload resolution. */
  jtl::option<handle> processor::gen(analyze::expr::recursion_reference_ref const expr,
                                     analyze::expr::function_arity const &)
  {
    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", munge(expr->fn_ctx->fn->name));
      return none;
    }
    return munge(expr->fn_ctx->fn->name);
  }

  jtl::option<handle> processor::gen(analyze::expr::named_recursion_ref const expr,
                                     analyze::expr::function_arity const &fn_arity)
  {
    handle ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("named_recursion")) };

    auto const &source_tmp(
      gen(jtl::ref<analyze::expr::recursion_reference>{ &expr->recursion_ref }, fn_arity));
    format_dynamic_call(source_tmp.unwrap().str(true),
                        ret_tmp.str(true),
                        expr->arg_exprs,
                        fn_arity);

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp.str(true));
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::let_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    auto const &ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("let")) };
    bool used_option{};

    auto const last_expr_type{ cpp_util::expression_type(
      expr->body->values[expr->body->values.size() - 1]) };

    auto const &type_name{ cpp_util::get_qualified_type_name(
      Cpp::GetNonReferenceType(last_expr_type)) };
    if(cpp_util::is_any_object(last_expr_type))
    {
      util::format_to(body_buffer, "{} {}{ }; {", type_name, ret_tmp);
    }
    else
    {
      used_option = true;
      util::format_to(body_buffer, "jtl::option<{}> {}{ }; {", type_name, ret_tmp);
    }

    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      auto const local_type{ cpp_util::expression_type(pair.second) };
      auto const &val_tmp(gen(pair.second, fn_arity));
      auto const &munged_name(runtime::munge(local.unwrap().binding->native_name));

      /* Every binding is wrapped in its own scope, to allow shadowing.
       *
       * Also, bindings are references to their value expression, rather than a copy.
       * This is important for C++ interop, since the we don't want to, and we may not
       * be able to, just copy stack-allocated C++ objects around willy nilly. */
      if(expr->is_loop)
      {
        if(cpp_util::is_any_object(local_type))
        {
          util::format_to(body_buffer,
                          "{ object_ref {}({}); ",
                          munged_name,
                          val_tmp.unwrap().str(true));
        }
        else
        {
          util::format_to(body_buffer, "{ auto {}({}); ", munged_name, val_tmp.unwrap().str(true));
        }
      }
      else
      {
        /* Local array refs should be turned into pointers so we can work with them more easily. */
        if(Cpp::IsArrayType(Cpp::GetNonReferenceType(local_type)))
        {
          util::format_to(body_buffer,
                          "{ {} {}({}); ",
                          cpp_util::get_qualified_type_name(Cpp::GetPointerType(
                            Cpp::GetArrayElementType(Cpp::GetNonReferenceType(local_type)))),
                          munged_name,
                          val_tmp.unwrap().str(false));
        }
        else
        {
          util::format_to(body_buffer,
                          "{ auto &&{}({}); ",
                          munged_name,
                          val_tmp.unwrap().str(false));
        }
      }
    }

    if(expr->is_loop)
    {
      util::format_to(body_buffer, "while(true){");
    }

    for(auto it(expr->body->values.begin()); it != expr->body->values.end();)
    {
      auto const &val_tmp(gen(*it, fn_arity));

      /* We ignore all values but the last. */
      if(++it == expr->body->values.end() && val_tmp.is_some())
      {
        /* The last expression tmp needs to be movable. */
        util::format_to(body_buffer,
                        "{} = std::move({});",
                        ret_tmp,
                        val_tmp.unwrap().str(expr->needs_box));

        if(expr->is_loop)
        {
          util::format_to(body_buffer, " break;");
        }
      }
    }
    for(auto const &_ : expr->pairs)
    {
      static_cast<void>(_);
      util::format_to(body_buffer, "}");
    }

    if(expr->is_loop)
    {
      util::format_to(body_buffer, "}");
    }

    util::format_to(body_buffer, "}");

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {}{};", ret_tmp, (used_option ? ".unwrap()" : ""));
      return none;
    }

    return util::format("{}{}", ret_tmp, (used_option ? ".unwrap()" : ""));
  }

  jtl::option<handle>
  processor::gen(analyze::expr::letfn_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    auto const &ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("letfn")) };
    bool used_option{};

    auto const last_expr_type{ cpp_util::expression_type(
      expr->body->values[expr->body->values.size() - 1]) };

    auto const &type_name{ cpp_util::get_qualified_type_name(
      Cpp::GetNonReferenceType(last_expr_type)) };
    if(cpp_util::is_any_object(last_expr_type))
    {
      util::format_to(body_buffer, "{} {}{ }; {", type_name, ret_tmp);
    }
    else
    {
      used_option = true;
      util::format_to(body_buffer, "jtl::option<{}> {}{ }; {", type_name, ret_tmp);
    }

    /* We don't handle shadowed bindings very well, so we can run into problems where our
     * codegen doesn't work. For letfn, we detect shadowed bindings and get around potential
     * assignment issues by just using an object_ref. This can be removed once we
     * properly give shadowed bindings individual local_binding entries or we have some other
     * mechanism for tracking them. */
    bool has_shadowed_bindings{};
    native_set<jtl::immutable_string> seen_names;
    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      auto const &name{ local.unwrap().binding->native_name };
      if(seen_names.contains(name))
      {
        has_shadowed_bindings = true;
        break;
      }
      seen_names.emplace(name);
    }

    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      auto const val_expr(llvm::cast<analyze::expr::function>(pair.second.data));
      auto const &munged_name(runtime::munge(local.unwrap().binding->native_name));
      auto const type_name{ (
        has_shadowed_bindings
          ? "jank::runtime::object_ref"
          : util::format("jank::runtime::oref<{}>", runtime::munge(val_expr->unique_name))) };
      util::format_to(body_buffer, "{ {} {};", type_name, munged_name);
    }

    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      auto const &val_tmp(gen(pair.second, fn_arity));
      auto const &munged_name(runtime::munge(local.unwrap().binding->native_name));

      util::format_to(body_buffer, "{} = {}; ", munged_name, val_tmp.unwrap().str(false));
    }

    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));

      auto const &munged_name(runtime::munge(local.unwrap().binding->native_name));
      auto const val_expr(llvm::cast<analyze::expr::function>(pair.second.data));
      for(auto const &capture_pair : val_expr->captures())
      {
        auto const &capture_name(runtime::munge(capture_pair.second->native_name));
        util::format_to(body_buffer, "{}->{} = {}; ", munged_name, capture_name, capture_name);
      }
    }

    for(auto it(expr->body->values.begin()); it != expr->body->values.end();)
    {
      auto const &val_tmp(gen(*it, fn_arity));

      /* We ignore all values but the last. */
      if(++it == expr->body->values.end() && val_tmp.is_some())
      {
        /* The last expression tmp needs to be movable. */
        util::format_to(body_buffer,
                        "{} = std::move({});",
                        ret_tmp,
                        val_tmp.unwrap().str(expr->needs_box));
      }
    }
    for(auto const &_ : expr->pairs)
    {
      static_cast<void>(_);
      util::format_to(body_buffer, "}");
    }

    util::format_to(body_buffer, "}");

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return {}{};", ret_tmp, (used_option ? ".unwrap()" : ""));
      return none;
    }

    return util::format("{}{}", ret_tmp, (used_option ? ".unwrap()" : ""));
  }

  jtl::option<handle>
  processor::gen(analyze::expr::do_ref const expr, analyze::expr::function_arity const &arity)
  {
    jtl::option<handle> last;
    for(auto const &form : expr->values)
    {
      last = gen(form, arity);
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

  jtl::option<handle>
  processor::gen(analyze::expr::if_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("if")));
    auto const expr_type{ cpp_util::expression_type(expr->then) };
    util::format_to(body_buffer,
                    "{} {}{ };",
                    cpp_util::get_qualified_type_name(expr_type),
                    ret_tmp);
    auto const &condition_tmp(gen(expr->condition, fn_arity));
    util::format_to(body_buffer,
                    "if(jank::runtime::truthy({})) {",
                    condition_tmp.unwrap().str(false));
    auto const &then_tmp(gen(expr->then, fn_arity));
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
      auto const &else_tmp(gen(expr->else_.unwrap(), fn_arity));
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

  jtl::option<handle>
  processor::gen(analyze::expr::throw_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    auto const &value_tmp(gen(expr->value, fn_arity));
    /* We static_cast to object_ref here, since we'll be trying to catch an object_ref in any
     * try/catch forms. This loses us our type info, but C++ doesn't do implicit conversions
     * when catching and we're not using inheritance. */
    util::format_to(body_buffer,
                    "throw static_cast<jank::runtime::object_ref>({});",
                    value_tmp.unwrap().str(true));
    return none;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::try_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    auto const has_catch{ expr->catch_body.is_some() };
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("try")));
    util::format_to(body_buffer, "object_ref {}{ };", ret_tmp);

    util::format_to(body_buffer, "{");
    if(expr->finally_body.is_some())
    {
      util::format_to(body_buffer, "jank::util::scope_exit const finally{ [&](){ ");
      gen(expr->finally_body.unwrap(), fn_arity);
      util::format_to(body_buffer, "} };");
    }

    if(has_catch)
    {
      util::format_to(body_buffer, "try {");
      auto const &body_tmp(gen(expr->body, fn_arity));
      if(body_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, body_tmp.unwrap().str(true));
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
                      runtime::munge(expr->catch_body.unwrap().sym->name));
      auto const &catch_tmp(gen(expr->catch_body.unwrap().body, fn_arity));
      if(catch_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, catch_tmp.unwrap().str(true));
      }
      if(expr->position == analyze::expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
      }
      util::format_to(body_buffer, "}");
    }
    else
    {
      auto const &body_tmp(gen(expr->body, fn_arity));
      if(body_tmp.is_some())
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, body_tmp.unwrap().str(true));
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
  processor::gen(analyze::expr::case_ref const expr, analyze::expr::function_arity const &fn_arity)
  {
    auto const is_tail{ expr->position == analyze::expression_position::tail };
    auto const &ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("case")) };

    util::format_to(body_buffer, "jank::runtime::object_ref {}{ };", ret_tmp);

    auto const &value_tmp{ gen(expr->value_expr, fn_arity) };

    util::format_to(body_buffer,
                    "switch(jank_shift_mask_case_integer({}.erase(), {}, {})) {",
                    value_tmp.unwrap().str(true),
                    expr->shift,
                    expr->mask);

    jank_debug_assert(expr->keys.size() == expr->exprs.size());
    for(usize i{}; i < expr->keys.size(); ++i)
    {
      util::format_to(body_buffer, "case {}: {", expr->keys[i]);

      auto const &case_tmp{ gen(expr->exprs[i], fn_arity) };
      if(!is_tail)
      {
        util::format_to(body_buffer, "{} = {};", ret_tmp, case_tmp.unwrap().str(true));
      }
      util::format_to(body_buffer, "break; }");
    }

    util::format_to(body_buffer, "default: {");

    auto const &default_tmp{ gen(expr->default_expr, fn_arity) };
    if(!is_tail)
    {
      util::format_to(body_buffer, "{} = {};", ret_tmp, default_tmp.unwrap().str(true));
    }

    util::format_to(body_buffer, "} }");

    if(is_tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(expr::cpp_raw_ref const expr, expr::function_arity const &)
  {
    util::format_to(deps_buffer, "{}\n", expr->code);

    if(expr->position == analyze::expression_position::tail)
    {
      util::format_to(body_buffer, "return jank::runtime::jank_nil;");
      return none;
    }
    return none;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_type_ref const, analyze::expr::function_arity const &)
  {
    throw std::runtime_error{ "cpp_type has no codegen" };
  }

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_value_ref const expr, analyze::expr::function_arity const &)
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

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_cast_ref const expr, analyze::expr::function_arity const &arity)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_cast")));
    auto const value_tmp{ gen(expr->value_expr, arity) };

    if(Cpp::IsVoid(expr->conversion_type))
    {
      if(expr->position == expression_position::tail)
      {
        util::format_to(body_buffer, "return jank::runtime::jank_nil;");
        return none;
      }
      return "jank::runtime::jank_nil";
    }

    util::format_to(
      body_buffer,
      "auto const {}{ jank::runtime::convert<{}>::{}({}) };",
      ret_tmp,
      cpp_util::get_qualified_type_name(
        Cpp::GetTypeWithoutCv(Cpp::GetNonReferenceType(expr->conversion_type))),
      (expr->policy == conversion_policy::into_object ? "into_object" : "from_object"),
      value_tmp.unwrap().str(true));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_call_ref const expr, analyze::expr::function_arity const &arity)
  {
    if(expr->source_expr->kind == expression_kind::cpp_value)
    {
      auto const source{ static_cast<expr::cpp_value *>(expr->source_expr.data) };
      auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_call")));

      native_vector<handle> arg_tmps;
      arg_tmps.reserve(expr->arg_exprs.size());
      for(auto const &arg_expr : expr->arg_exprs)
      {
        arg_tmps.emplace_back(gen(arg_expr, arity).unwrap());
      }

      auto const is_void{ Cpp::IsVoid(Cpp::GetFunctionReturnType(source->scope)) };

      if(is_void)
      {
        util::format_to(body_buffer, "jank::runtime::object_ref const {};", ret_tmp);
      }
      else
      {
        util::format_to(body_buffer, "auto &&{}{ ", ret_tmp);
      }

      util::format_to(body_buffer, "{}(", Cpp::GetQualifiedCompleteName(source->scope));

      bool need_comma{};
      for(usize arg_idx{}; arg_idx < expr->arg_exprs.size(); ++arg_idx)
      {
        auto const arg_expr{ expr->arg_exprs[arg_idx] };
        auto const arg_type{ cpp_util::expression_type(arg_expr) };
        auto const param_type{ Cpp::GetFunctionArgType(source->scope, arg_idx) };
        auto const &arg_tmp{ arg_tmps[arg_idx] };

        if(need_comma)
        {
          util::format_to(body_buffer, ", ");
        }
        util::format_to(body_buffer, "{}", arg_tmp.str(true));
        if(param_type && Cpp::IsPointerType(param_type) && cpp_util::is_any_object(arg_type))
        {
          util::format_to(body_buffer, ".erase()");
        }
        need_comma = true;
      }

      util::format_to(body_buffer, ")");

      if(!is_void)
      {
        util::format_to(body_buffer, "};");
      }
      else
      {
        util::format_to(body_buffer, ";");
      }

      if(expr->position == expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
        return none;
      }

      return ret_tmp;
    }
    else
    {
      auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_call")));

      auto const source_tmp{ gen(expr->source_expr, arity).unwrap() };

      native_vector<handle> arg_tmps;
      arg_tmps.reserve(expr->arg_exprs.size());
      for(auto const &arg_expr : expr->arg_exprs)
      {
        arg_tmps.emplace_back(gen(arg_expr, arity).unwrap());
      }

      auto const is_void{ Cpp::IsVoid(expr->type) };

      if(is_void)
      {
        util::format_to(body_buffer, "jank::runtime::object_ref const {};", ret_tmp);
      }
      else
      {
        util::format_to(body_buffer, "auto &&{}{ ", ret_tmp);
      }

      util::format_to(body_buffer, "{}(", source_tmp.str(true));

      bool need_comma{};
      for(auto const &arg_tmp : arg_tmps)
      {
        if(need_comma)
        {
          util::format_to(body_buffer, ", ");
        }
        util::format_to(body_buffer, "{}", arg_tmp.str(true));
        need_comma = true;
      }

      util::format_to(body_buffer, ")");

      if(!is_void)
      {
        util::format_to(body_buffer, "};");
      }
      else
      {
        util::format_to(body_buffer, ";");
      }

      if(expr->position == expression_position::tail)
      {
        util::format_to(body_buffer, "return {};", ret_tmp);
        return none;
      }

      return ret_tmp;
    }
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_constructor_call_ref const expr,
                                     analyze::expr::function_arity const &arity)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_ctor")));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, arity).unwrap());
    }

    if(expr->arg_exprs.empty())
    {
      util::format_to(body_buffer,
                      "{} {}{ };",
                      cpp_util::get_qualified_type_name(expr->type),
                      ret_tmp);
      return ret_tmp;
    }

    util::format_to(body_buffer, "{} {}{ ", cpp_util::get_qualified_type_name(expr->type), ret_tmp);

    if(!expr->arg_exprs.empty())
    {
      auto const arg_type{ cpp_util::expression_type(expr->arg_exprs[0]) };
      bool needs_conversion{};
      jtl::immutable_string conversion_type;
      if(cpp_util::is_any_object(expr->type) && !cpp_util::is_any_object(arg_type))
      {
        needs_conversion = true;
        conversion_type = "into_object";
      }
      else if(!cpp_util::is_any_object(expr->type) && cpp_util::is_any_object(arg_type))
      {
        needs_conversion = true;
        conversion_type = "from_object";
      }

      if(needs_conversion)
      {
        util::format_to(body_buffer,
                        "jank::runtime::convert<{}>::{}({}.get())",
                        cpp_util::get_qualified_type_name(expr->type),
                        conversion_type,
                        arg_tmps[0].str(false));
      }
      else
      {
        auto const needs_static_cast{ expr->type != arg_type && expr->arg_exprs.size() == 1 };
        if(needs_static_cast)
        {
          util::format_to(body_buffer,
                          "static_cast<{}>(",
                          cpp_util::get_qualified_type_name(expr->type));
        }

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

        if(needs_static_cast)
        {
          util::format_to(body_buffer, ")");
        }
      }
    }

    util::format_to(body_buffer, " };");

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }
    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_member_call_ref const expr,
                                     analyze::expr::function_arity const &arity)
  {
    auto const fn_name{ Cpp::GetName(expr->fn) };
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string(fn_name)));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, arity).unwrap());
    }

    auto const is_void{ Cpp::IsVoid(Cpp::GetFunctionReturnType(expr->fn)) };

    if(is_void)
    {
      util::format_to(body_buffer, "jank::runtime::object_ref {}{ };", ret_tmp);
      util::format_to(
        body_buffer,
        "{}{}{}(",
        arg_tmps[0].str(false),
        (Cpp::IsPointerType(cpp_util::expression_type(expr->arg_exprs[0])) ? "->" : "."),
        fn_name);
    }
    else
    {
      util::format_to(
        body_buffer,
        "auto &&{}{ {}{}{}(",
        ret_tmp,
        arg_tmps[0].str(false),
        (Cpp::IsPointerType(cpp_util::expression_type(expr->arg_exprs[0])) ? "->" : "."),
        fn_name);
    }

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

    if(is_void)
    {
      util::format_to(body_buffer, ");");
    }
    else
    {
      util::format_to(body_buffer, ") };");
    }

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_member_access_ref const expr,
                                     analyze::expr::function_arity const &arity)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string(expr->name)));
    auto obj_tmp(gen(expr->obj_expr, arity));

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
                                     analyze::expr::function_arity const &arity)
  {
    auto ret_tmp(runtime::munge(__rt_ctx->unique_namespaced_string("cpp_operator")));

    native_vector<handle> arg_tmps;
    arg_tmps.reserve(expr->arg_exprs.size());
    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_tmps.emplace_back(gen(arg_expr, arity).unwrap());
    }

    auto const op_name{ cpp_util::operator_name(static_cast<Cpp::Operator>(expr->op)).unwrap() };

    if(expr->arg_exprs.size() == 1)
    {
      util::format_to(body_buffer, "auto &&{}( {}{} );", ret_tmp, op_name, arg_tmps[0].str(false));
    }
    else if(op_name == "aget")
    {
      util::format_to(body_buffer,
                      "auto &&{}( {}[{}] );",
                      ret_tmp,
                      arg_tmps[0].str(false),
                      arg_tmps[1].str(false));
    }
    else
    {
      util::format_to(body_buffer,
                      "auto &&{}( {} {} {} );",
                      ret_tmp,
                      arg_tmps[0].str(false),
                      op_name,
                      arg_tmps[1].str(false));
    }

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_box_ref const expr, analyze::expr::function_arity const &arity)
  {
    auto ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("cpp_box")) };
    auto value_tmp{ gen(expr->value_expr, arity) };
    auto const value_expr_type{ cpp_util::expression_type(expr->value_expr) };
    auto const type_str{ Cpp::GetTypeAsString(
      Cpp::GetCanonicalType(Cpp::GetNonReferenceType(value_expr_type))) };

    util::format_to(
      body_buffer,
      "auto {}{ jank::runtime::make_box<jank::runtime::obj::opaque_box>({}, \"{}\") };\n",
      ret_tmp,
      value_tmp.unwrap().str(false),
      type_str);

    auto const meta{ runtime::source_to_meta(expr->source) };
    util::format_to(body_buffer,
                    "jank::runtime::reset_meta({}, jank::runtime::__rt_ctx->read_string(\"{}\"));",
                    ret_tmp,
                    util::escape(runtime::to_code_string(meta)));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_unbox_ref const expr,
                                     analyze::expr::function_arity const &arity)
  {
    auto ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("cpp_unbox")) };
    auto value_tmp{ gen(expr->value_expr, arity) };
    auto const type_name{ cpp_util::get_qualified_type_name(expr->type) };
    auto const meta{ runtime::source_to_meta(expr->source) };

    util::format_to(body_buffer,
                    "auto {}{ "
                    "static_cast<{}>(jank_unbox_with_source(\"{}\", {}.data, "
                    "jank::runtime::__rt_ctx->read_string(\"{}\").data)"
                    ") };",
                    ret_tmp,
                    type_name,
                    type_name,
                    value_tmp.unwrap().str(false),
                    util::escape(runtime::to_code_string(meta)));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle>
  processor::gen(analyze::expr::cpp_new_ref const expr, analyze::expr::function_arity const &arity)
  {
    auto ret_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("cpp_new")) };
    auto finalizer_tmp{ runtime::munge(__rt_ctx->unique_namespaced_string("finalizer")) };
    auto value_tmp{ gen(expr->value_expr, arity) };

    auto const type_name{ cpp_util::get_qualified_type_name(expr->type) };
    auto const needs_finalizer{ !Cpp::IsTriviallyDestructible(expr->type) };

    if(needs_finalizer)
    {
      util::format_to(body_buffer,
                      "using T = {};\n"
                      "static auto const {}{ "
                      "[](void * const obj, void *){"
                      "reinterpret_cast<T*>(obj)->~T();"
                      "} };",
                      type_name,
                      finalizer_tmp);
    }

    util::format_to(body_buffer,
                    "auto {}{ "
                    "new (GC{}) {}{ {} }"
                    " };",
                    ret_tmp,
                    (needs_finalizer ? ", " + finalizer_tmp : ""),
                    type_name,
                    value_tmp.unwrap().str(false));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return {};", ret_tmp);
      return none;
    }

    return ret_tmp;
  }

  jtl::option<handle> processor::gen(analyze::expr::cpp_delete_ref const expr,
                                     analyze::expr::function_arity const &arity)
  {
    auto value_tmp{ gen(expr->value_expr, arity).unwrap() };
    auto const value_type{ Cpp::GetPointeeType(cpp_util::expression_type(expr->value_expr)) };
    auto const type_name{ cpp_util::get_qualified_type_name(value_type) };
    auto const needs_finalizer{ !Cpp::IsTriviallyDestructible(value_type) };

    /* Calling GC_free won't trigger the finalizer. Not sure why, but it's explicitly
     * documented in bdwgc. So, we'll invoke it manually if needed, prior to GC_free. */
    if(needs_finalizer)
    {
      util::format_to(body_buffer,
                      "using T = {};\n"
                      "{}->~T();",
                      type_name,
                      value_tmp.str(false));
    }

    util::format_to(body_buffer, "GC_free({});", value_tmp.str(false));

    if(expr->position == expression_position::tail)
    {
      util::format_to(body_buffer, "return jank::runtime::jank_nil;");
      return none;
    }

    return "jank::runtime::jank_nil";
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

          /* TODO: Typed lifted constants. */
          util::format_to(header_buffer,
                          "{} const {};",
                          detail::gen_constant_type(v.second.data, true),
                          runtime::munge(v.second.native_name));
        }

        /* TODO: More useful types here. */
        for(auto const &v : arity.frame->captures)
        {
          if(used_captures.contains(v.first->to_hash()))
          {
            continue;
          }
          used_captures.emplace(v.first->to_hash());

          /* Captures aren't const since they could be late-assigned, in the case of a letfn. */
          util::format_to(header_buffer,
                          "jank::runtime::object_ref {};",
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
        util::format_to(body_buffer,
                        "jank::runtime::object_ref const {}{ this };",
                        runtime::munge(root_fn->name));
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
        gen(form, arity);
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
      util::format_to(footer_buffer, "jank_ns_intern_c(\"{}\");", module);
      util::format_to(footer_buffer,
                      "return {}::{}{ }.call().erase();",
                      runtime::module::module_to_native_ns(module),
                      runtime::munge(struct_name.name));
      util::format_to(footer_buffer, "}");
    }
  }

  jtl::immutable_string processor::expression_str()
  {
    auto const module_ns(runtime::module::module_to_native_ns(module));

    if(!generated_expression)
    {
      util::format_to(expression_buffer,
                      "jank::runtime::make_box<{}>(",
                      runtime::module::nest_native_ns(module_ns, runtime::munge(struct_name.name)));

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
            auto const local_type{ originating_local.unwrap().binding->type };
            auto const needs_conversion{ !cpp_util::is_any_object(local_type) };

            if(needs_conversion)
            {
              util::format_to(expression_buffer,
                              "{} jank::runtime::convert<{}>::{}({})",
                              (need_comma ? "," : ""),
                              cpp_util::get_qualified_type_name(local_type),
                              "into_object",
                              h.str(true));
            }
            else
            {
              util::format_to(expression_buffer, "{} {}", (need_comma ? "," : ""), h.str(true));
            }
          }
          need_comma = true;
        }
      }

      util::format_to(expression_buffer, ")");

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }

  /* TODO: Not sure if we want any of this. The module dependency loading feels wrong,
   * since it should be tied to calls to require instead. */
  jtl::immutable_string processor::module_init_str(jtl::immutable_string const &module)
  {
    jtl::string_builder module_buffer;

    util::format_to(module_buffer, "namespace {} {", runtime::module::module_to_native_ns(module));

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
    for(auto const &dep : __rt_ctx->module_dependencies[module])
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
