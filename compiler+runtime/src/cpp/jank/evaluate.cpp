#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/jit/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/profile/time.hpp>
#include <jank/util/scope_exit.hpp>

namespace jank::evaluate
{
  using namespace jank::runtime;
  using namespace jank::analyze;

  /* TODO: Move postwalk into the nodes. */
  template <typename T, typename F>
  requires std::is_base_of_v<expression_base, T>
  static void walk(T const &expr, F const &f)
  {
    if constexpr(std::same_as<T, expr::call<expression>>)
    {
      walk(expr.source_expr, f);
      for(auto const &form : expr.arg_exprs)
      {
        walk(form, f);
      }
    }
    else if constexpr(std::same_as<T, expr::def<expression>>)
    {
      if(expr.value.is_some())
      {
        walk(expr.value.unwrap(), f);
      }
    }
    else if constexpr(std::same_as<T, expr::if_<expression>>)
    {
      walk(expr.condition, f);
      walk(expr.then, f);
      if(expr.else_.is_some())
      {
        walk(expr.else_.unwrap(), f);
      }
    }
    else if constexpr(std::same_as<T, expr::do_<expression>>)
    {
      for(auto const &form : expr.values)
      {
        walk(form, f);
      }
    }
    else if constexpr(std::same_as<T, expr::let<expression>>)
    {
      walk(expr.body, f);
    }
    else if constexpr(std::same_as<T, expr::throw_<expression>>)
    {
      walk(expr.value, f);
    }
    else if constexpr(std::same_as<T, expr::try_<expression>>)
    {
      walk(expr.body, f);
      if(expr.catch_body.is_some())
      {
        walk(expr.catch_body.unwrap().body, f);
      }
      if(expr.finally_body.is_some())
      {
        walk(expr.finally_body.unwrap(), f);
      }
    }
    else if constexpr(std::same_as<T, expr::list<expression>>)
    {
      for(auto const &form : expr.data_exprs)
      {
        walk(form, f);
      }
    }
    else if constexpr(std::same_as<T, expr::vector<expression>>)
    {
      for(auto const &form : expr.data_exprs)
      {
        walk(form, f);
      }
    }
    else if constexpr(std::same_as<T, expr::set<expression>>)
    {
      for(auto const &form : expr.data_exprs)
      {
        walk(form, f);
      }
    }
    else if constexpr(std::same_as<T, expr::map<expression>>)
    {
      for(auto const &form : expr.data_exprs)
      {
        walk(form.first, f);
        walk(form.second, f);
      }
    }
    /* TODO: function */

    f(expr);
  }

  template <typename F>
  static void walk(expression_ptr const expr, F const &f)
  {
    boost::apply_visitor([&](auto &typed_expr) { walk(typed_expr, f); }, expr->data);
  }

  template <typename F>
  static void walk(expr::function_arity<expression> const &arity, F const &f)
  {
    walk(arity.body, f);
  }

  /* Some expressions don't make sense to eval outright and aren't fns that can be JIT compiled.
   * For those, we wrap them in a fn expression and then JIT compile and call them.
   *
   * There's an oddity here, since that expr wouldn't've been analyzed within a fn frame, so
   * its lifted vars/constants, for example, aren't in a fn frame. Instead, they're put in the
   * root frame. So, when wrapping this expr, we give the fn the root frame, but change its
   * type to a fn frame. */
  template <typename E>
  static expression_ptr wrap_expression(E expr,
                                        native_persistent_string const &name,
                                        native_vector<obj::symbol_ptr> params)
  {
    auto ret(make_box<expression>(expr::function<expression>{}));
    auto &fn(boost::get<expr::function<expression>>(ret->data));
    expr::function_arity<expression> arity;
    fn.name = name;
    fn.unique_name = __rt_ctx->unique_string(fn.name);
    fn.meta = obj::persistent_hash_map::empty();

    auto const &closest_fn_frame(local_frame::find_closest_fn_frame(*expr.frame));

    arity.frame = make_box<local_frame>(local_frame::frame_type::fn, *__rt_ctx, expr.frame->parent);
    expr.frame->parent = arity.frame;
    fn.frame = arity.frame->parent.unwrap_or(arity.frame);
    fn.frame->lift_constant(fn.meta);
    arity.frame->fn_ctx = make_box<expr::function_context>();
    arity.frame->fn_ctx->name = fn.name;
    arity.frame->fn_ctx->unique_name = fn.unique_name;
    arity.frame->fn_ctx->fn = ret;
    arity.fn_ctx = arity.frame->fn_ctx;

    arity.frame->lifted_constants = closest_fn_frame.lifted_constants;

    arity.params = std::move(params);
    arity.fn_ctx->param_count = arity.params.size();
    for(auto const sym : arity.params)
    {
      arity.frame->locals.emplace(sym, local_binding{ sym, none, arity.frame });
    }

    /* We can't just assign the position here, since we need the position to propagate
     * downward. For example, if this expr is a let, setting its position to tail
     * wouldn't affect the last form of its body, which should also be in tail position.
     *
     * This is what propagation does. */
    expr.propagate_position(expression_position::tail);

    /* TODO: Avoid allocation by using existing ptr. */
    arity.body.values.push_back(make_box<expression>(expr));
    arity.body.frame = arity.frame;

    walk(arity, [&](auto const &form) {
      using T = std::decay_t<decltype(form)>;

      if constexpr(std::same_as<T, expr::local_reference>)
      {
        auto found_local(expr.frame->find_local_or_capture(form.name));
        if(found_local && !found_local.unwrap().crossed_fns.empty())
        {
          arity.frame->captures[form.name] = found_local.unwrap().binding;
        }
      }
    });

    fn.arities.emplace_back(std::move(arity));

    return ret;
  }

  /* TODO: Expression wrapping makes sense in analyze, not eval. We use it all over the place. */
  expression_ptr wrap_expressions(native_vector<expression_ptr> const &exprs,
                                  processor const &an_prc,
                                  native_persistent_string const &name)
  {
    if(exprs.empty())
    {
      return wrap_expression(
        expr::primitive_literal<expression>{
          expression_base{ {}, expression_position::tail, an_prc.root_frame, true },
          obj::nil::nil_const(),
      },
        name,
        {});
    }
    else
    {
      /* We'll cheat a little and build a fn using just the first expression. Then we can just
       * add the rest. I'd rather do this than duplicate all of the wrapping logic. */
      auto ret(wrap_expression(exprs[0], name, {}));
      auto &fn(boost::get<expr::function<expression>>(ret->data));
      auto &body(fn.arities[0].body.values);
      /* We normally wrap one expression, which is a return statement, but we'll be potentially
       * adding more, so let's not make assumptions yet. */
      body[0]->propagate_position(expression_position::statement);

      if(exprs.size() > 1)
      {
        for(auto it(exprs.begin() + 1); it != exprs.end(); ++it)
        {
          auto &expr(*it);
          expr->propagate_position(expression_position::statement);
          body.emplace_back(expr);
        }
      }

      /* Finally, mark the last body item as our return. */
      auto const last_body_index(body.size() - 1);
      body[last_body_index]->propagate_position(expression_position::tail);

      return ret;
    }
  }

  expression_ptr wrap_expression(expression_ptr const expr,
                                 native_persistent_string const &name,
                                 native_vector<obj::symbol_ptr> params)
  {
    return boost::apply_visitor(
      [&](auto const &typed_expr) { return wrap_expression(typed_expr, name, std::move(params)); },
      expr->data);
  }

  object_ptr eval(expression_ptr const &ex)
  {
    profile::timer const timer{ "eval ast node" };
    object_ptr ret{};
    boost::apply_visitor([&ret](auto const &typed_ex) { ret = eval(typed_ex); }, ex->data);

    assert(ret);
    return ret;
  }

  object_ptr eval(expr::def<expression> const &expr)
  {
    auto var(__rt_ctx->intern_var(expr.name).expect_ok());
    var->meta = expr.name->meta;

    auto const meta(var->meta.unwrap_or(obj::nil::nil_const()));
    auto const dynamic(get(meta, __rt_ctx->intern_keyword("dynamic").expect_ok()));
    var->set_dynamic(truthy(dynamic));

    if(expr.value.is_none())
    {
      return var;
    }

    auto const evaluated_value(eval(expr.value.unwrap()));
    var->bind_root(evaluated_value);

    return var;
  }

  object_ptr eval(expr::var_deref<expression> const &expr)
  {
    auto const var(__rt_ctx->find_var(expr.qualified_name));
    return var.unwrap()->deref();
  }

  object_ptr eval(expr::var_ref<expression> const &expr)
  {
    auto const var(__rt_ctx->find_var(expr.qualified_name));
    return var.unwrap();
  }

  object_ptr eval(expr::call<expression> const &expr)
  {
    auto source(eval(expr.source_expr));
    if(source->type == object_type::var)
    {
      source = deref(source);
    }

    return visit_object(
      [&](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(std::is_base_of_v<behavior::callable, T>)
        {
          native_vector<object_ptr> arg_vals;
          arg_vals.reserve(expr.arg_exprs.size());
          for(auto const &arg_expr : expr.arg_exprs)
          {
            arg_vals.emplace_back(eval(arg_expr));
          }

          switch(arg_vals.size())
          {
            case 0:
              return dynamic_call(source);
            case 1:
              return dynamic_call(source, arg_vals[0]);
            case 2:
              return dynamic_call(source, arg_vals[0], arg_vals[1]);
            case 3:
              return dynamic_call(source, arg_vals[0], arg_vals[1], arg_vals[2]);
            case 4:
              return dynamic_call(source, arg_vals[0], arg_vals[1], arg_vals[2], arg_vals[3]);
            case 5:
              return dynamic_call(source,
                                  arg_vals[0],
                                  arg_vals[1],
                                  arg_vals[2],
                                  arg_vals[3],
                                  arg_vals[4]);
            case 6:
              return dynamic_call(source,
                                  arg_vals[0],
                                  arg_vals[1],
                                  arg_vals[2],
                                  arg_vals[3],
                                  arg_vals[4],
                                  arg_vals[5]);
            case 7:
              return dynamic_call(source,
                                  arg_vals[0],
                                  arg_vals[1],
                                  arg_vals[2],
                                  arg_vals[3],
                                  arg_vals[4],
                                  arg_vals[5],
                                  arg_vals[6]);
            case 8:
              return dynamic_call(source,
                                  arg_vals[0],
                                  arg_vals[1],
                                  arg_vals[2],
                                  arg_vals[3],
                                  arg_vals[4],
                                  arg_vals[5],
                                  arg_vals[6],
                                  arg_vals[7]);
            case 9:
              return dynamic_call(source,
                                  arg_vals[0],
                                  arg_vals[1],
                                  arg_vals[2],
                                  arg_vals[3],
                                  arg_vals[4],
                                  arg_vals[5],
                                  arg_vals[6],
                                  arg_vals[7],
                                  arg_vals[8]);
            case 10:
              return dynamic_call(source,
                                  arg_vals[0],
                                  arg_vals[1],
                                  arg_vals[2],
                                  arg_vals[3],
                                  arg_vals[4],
                                  arg_vals[5],
                                  arg_vals[6],
                                  arg_vals[7],
                                  arg_vals[8],
                                  arg_vals[9]);
            default:
              {
                return dynamic_call(source,
                                    arg_vals[0],
                                    arg_vals[1],
                                    arg_vals[2],
                                    arg_vals[3],
                                    arg_vals[4],
                                    arg_vals[5],
                                    arg_vals[6],
                                    arg_vals[7],
                                    arg_vals[8],
                                    arg_vals[9],
                                    try_object<obj::persistent_list>(arg_vals[10]));
              }
          }
        }
        else if constexpr(std::same_as<T, obj::persistent_hash_set>
                          || std::same_as<T, obj::transient_vector>)
        {
          auto const s(expr.arg_exprs.size());
          if(s != 1)
          {
            throw std::runtime_error{
              fmt::format("invalid call with {} args to: {}", s, typed_source->to_string())
            };
          }
          return typed_source->call(eval(expr.arg_exprs[0]));
        }
        else if constexpr(std::same_as<T, obj::keyword> || std::same_as<T, obj::persistent_hash_map>
                          || std::same_as<T, obj::persistent_array_map>
                          || std::same_as<T, obj::transient_hash_set>)
        {
          auto const s(expr.arg_exprs.size());
          switch(s)
          {
            case 1:
              return typed_source->call(eval(expr.arg_exprs[0]));
            case 2:
              return typed_source->call(eval(expr.arg_exprs[0]), eval(expr.arg_exprs[1]));
            default:
              throw std::runtime_error{
                fmt::format("invalid call with {} args to: {}", s, typed_source->to_string())
              };
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 0 args to: {}",
                                                expr.arg_exprs.size(),
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr eval(expr::primitive_literal<expression> const &expr)
  {
    if(expr.data->type == object_type::keyword)
    {
      auto const d(expect_object<obj::keyword>(expr.data));
      return __rt_ctx->intern_keyword(d->sym->ns, d->sym->name).expect_ok();
    }
    return expr.data;
  }

  object_ptr eval(expr::list<expression> const &expr)
  {
    native_vector<object_ptr> ret;
    for(auto const &e : expr.data_exprs)
    {
      ret.emplace_back(eval(e));
    }

    runtime::detail::native_persistent_list const npl{ ret.rbegin(), ret.rend() };
    if(expr.meta.is_some())
    {
      return make_box<obj::persistent_list>(expr.meta.unwrap(), std::move(npl));
    }
    else
    {
      return make_box<obj::persistent_list>(std::move(npl));
    }
  }

  object_ptr eval(expr::vector<expression> const &expr)
  {
    runtime::detail::native_transient_vector ret;
    for(auto const &e : expr.data_exprs)
    {
      ret.push_back(eval(e));
    }
    if(expr.meta.is_some())
    {
      return make_box<obj::persistent_vector>(expr.meta.unwrap(), ret.persistent());
    }
    else
    {
      return make_box<obj::persistent_vector>(ret.persistent());
    }
  }

  object_ptr eval(expr::map<expression> const &expr)
  {
    auto const size(expr.data_exprs.size());
    if(size <= obj::persistent_array_map::max_size)
    {
      auto const array_box(make_array_box<object_ptr>(size * 2));
      size_t i{};
      for(auto const &e : expr.data_exprs)
      {
        array_box.data[i++] = eval(e.first);
        array_box.data[i++] = eval(e.second);
      }

      if(expr.meta.is_some())
      {
        return make_box<obj::persistent_array_map>(expr.meta.unwrap(),
                                                   runtime::detail::in_place_unique{},
                                                   array_box,
                                                   size * 2);
      }
      else
      {
        return make_box<obj::persistent_array_map>(runtime::detail::in_place_unique{},
                                                   array_box,
                                                   size * 2);
      }
    }
    else
    {
      runtime::detail::native_transient_hash_map trans;
      for(auto const &e : expr.data_exprs)
      {
        trans.insert({ eval(e.first), eval(e.second) });
      }

      if(expr.meta.is_some())
      {
        return make_box<obj::persistent_hash_map>(expr.meta.unwrap(), trans.persistent());
      }
      else
      {
        return make_box<obj::persistent_hash_map>(trans.persistent());
      }
    }
  }

  object_ptr eval(expr::set<expression> const &expr)
  {
    runtime::detail::native_transient_hash_set ret;
    for(auto const &e : expr.data_exprs)
    {
      ret.insert(eval(e));
    }
    if(expr.meta.is_some())
    {
      return make_box<obj::persistent_hash_set>(expr.meta.unwrap(), std::move(ret).persistent());
    }
    else
    {
      return make_box<obj::persistent_hash_set>(std::move(ret).persistent());
    }
  }

  object_ptr eval(expr::local_reference const &)
  /* Doesn't make sense to eval these, since let is wrapped in a fn and JIT compiled. */
  {
    throw make_box("unsupported eval: local_reference");
  }

  object_ptr eval(expr::function<expression> const &expr)
  {
    auto const &module(
      module::nest_module(expect_object<ns>(__rt_ctx->current_ns_var->deref())->to_string(),
                          munge(expr.unique_name)));

    auto const wrapped_expr(evaluate::wrap_expression(expr, "repl_fn", {}));
    codegen::llvm_processor cg_prc{ wrapped_expr, module, codegen::compilation_target::eval };
    cg_prc.gen().expect_ok();

    {
      profile::timer const timer{ fmt::format("ir jit compile {}", expr.name) };
      __rt_ctx->jit_prc.load_ir_module(std::move(cg_prc.ctx->module),
                                       std::move(cg_prc.ctx->llvm_ctx));

      auto const fn(
        __rt_ctx->jit_prc
          .find_symbol<object *(*)()>(fmt::format("{}_0", munge(cg_prc.root_fn.unique_name)))
          .expect_ok());
      return fn();
    }
  }

  object_ptr eval(expr::recur<expression> const &)
  /* This will always be in a fn or loop, which will be JIT compiled. */
  {
    throw make_box("unsupported eval: recur");
  }

  object_ptr eval(expr::recursion_reference<expression> const &)
  /* This will always be in a fn, which will be JIT compiled. */
  {
    throw make_box("unsupported eval: recursion_reference");
  }

  object_ptr eval(expr::named_recursion<expression> const &)
  /* This will always be in a fn, which will be JIT compiled. */
  {
    throw make_box("unsupported eval: named_recursion");
  }

  object_ptr eval(expr::do_<expression> const &expr)
  {
    object_ptr ret{ obj::nil::nil_const() };
    for(auto const &form : expr.values)
    {
      ret = eval(form);
    }
    return ret;
  }

  object_ptr eval(expr::let<expression> const &expr)
  {
    return dynamic_call(eval(wrap_expression(expr, "let", {})));
  }

  object_ptr eval(expr::if_<expression> const &expr)
  {
    auto const condition(eval(expr.condition));
    if(truthy(condition))
    {
      return eval(expr.then);
    }
    else if(expr.else_.is_some())
    {
      return eval(expr.else_.unwrap());
    }
    return obj::nil::nil_const();
  }

  object_ptr eval(expr::throw_<expression> const &expr)
  {
    throw eval(expr.value);
  }

  object_ptr eval(expr::try_<expression> const &expr)
  {
    util::scope_exit const finally{ [=]() {
      if(expr.finally_body)
      {
        eval(expr.finally_body.unwrap());
      }
    } };

    if(!expr.catch_body)
    {
      return eval(expr.body);
    }
    try
    {
      return eval(expr.body);
    }
    catch(object_ptr const e)
    {
      return dynamic_call(eval(wrap_expression(make_box<expression>(expr.catch_body.unwrap().body),
                                               "catch",
                                               { expr.catch_body.unwrap().sym })),
                          e);
    }
  }
}
