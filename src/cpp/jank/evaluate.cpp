#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/codegen/llvm/processor.hpp>
#include <jank/jit/processor.hpp>
#include <jank/evaluate.hpp>

namespace jank::evaluate
{
  /* Some expressions don't make sense to eval outright and aren't fns that can be JIT compiled.
   * For those, we wrap them in a fn expression and then JIT compile and call them.
   *
   * There's an oddity here, since that expr wouldn't've been analyzed within a fn frame, so
   * its lifted vars/constants, for example, aren't in a fn frame. Instead, they're put in the
   * root frame. So, when wrapping this expr, we give the fn the root frame, but change its
   * type to a fn frame. */
  template <typename E>
  analyze::expr::function<analyze::expression> wrap_expression(E expr)
  {
    analyze::expr::function<analyze::expression> wrapper;
    analyze::expr::function_arity<analyze::expression> arity;

    arity.frame = expr.frame;
    while(arity.frame->parent.is_some())
    {
      arity.frame = arity.frame->parent.unwrap();
    }
    arity.frame->type = analyze::local_frame::frame_type::fn;
    expr.expr_type = analyze::expression_type::return_statement;
    /* TODO: Avoid allocation by using existing ptr. */
    arity.body.body.push_back(make_box<analyze::expression>(expr));
    arity.fn_ctx = make_box<analyze::expr::function_context>();
    arity.body.frame = arity.frame;

    wrapper.arities.emplace_back(std::move(arity));
    wrapper.frame = expr.frame;
    wrapper.name = runtime::context::unique_string("repl_fn");

    return wrapper;
  }

  analyze::expr::function<analyze::expression>
  wrap_expressions(native_vector<analyze::expression_ptr> const &exprs,
                   analyze::processor const &an_prc)
  {
    if(exprs.empty())
    {
      return wrap_expression(analyze::expr::primitive_literal<analyze::expression>{
        analyze::expression_base{{},
                                 analyze::expression_type::return_statement,
                                 an_prc.root_frame,
                                 true},
        runtime::obj::nil::nil_const()
      });
    }
    else
    {
      /* We'll cheat a little and build a fn using just the first expression. Then we can just
       * add the rest. I'd rather do this than duplicate all of the wrapping logic. */
      auto ret(wrap_expression(exprs[0]));
      auto &body(ret.arities[0].body.body);
      /* We normally wrap one expression, which is a return statement, but we'll be potentially
       * adding more, so let's not make assumptions yet. */
      body[0]->get_base()->expr_type = analyze::expression_type::statement;

      for(auto const &expr : exprs)
      {
        body.emplace_back(expr);
      }

      /* Finally, mark the last body item as our return. */
      auto const last_body_index(body.size() - 1);
      body[last_body_index]->get_base()->expr_type = analyze::expression_type::return_statement;

      return ret;
    }
  }

  analyze::expr::function<analyze::expression> wrap_expression(analyze::expression_ptr const expr)
  {
    return boost::apply_visitor([](auto const &typed_expr) { return wrap_expression(typed_expr); },
                                expr->data);
  }

  runtime::object_ptr
  eval(runtime::context &rt_ctx, jit::processor const &jit_prc, analyze::expression_ptr const &ex)
  {
    runtime::object_ptr ret{};
    boost::apply_visitor(
      [&rt_ctx, &jit_prc, &ret](auto const &typed_ex) { ret = eval(rt_ctx, jit_prc, typed_ex); },
      ex->data);

    assert(ret);
    return ret;
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::def<analyze::expression> const &expr)
  {
    auto var(rt_ctx.intern_var(expr.name).expect_ok());
    if(expr.value.is_none())
    {
      return var;
    }

    auto const evaluated_value(eval(rt_ctx, jit_prc, expr.value.unwrap()));
    var->bind_root(evaluated_value);
    return var;
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &,
                           analyze::expr::var_deref<analyze::expression> const &expr)
  {
    auto const var(rt_ctx.find_var(expr.qualified_name));
    return var.unwrap()->deref();
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &,
                           analyze::expr::var_ref<analyze::expression> const &expr)
  {
    auto const var(rt_ctx.find_var(expr.qualified_name));
    return var.unwrap();
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::call<analyze::expression> const &expr)
  {
    auto const source(eval(rt_ctx, jit_prc, expr.source_expr));
    return runtime::visit_object(
      [&](auto const typed_source) -> runtime::object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(std::is_base_of_v<runtime::behavior::callable, T>)
        {
          native_vector<runtime::object_ptr> arg_vals;
          arg_vals.reserve(expr.arg_exprs.size());
          for(auto const &arg_expr : expr.arg_exprs)
          {
            arg_vals.emplace_back(eval(rt_ctx, jit_prc, arg_expr));
          }

          /* TODO: Use apply_to */
          switch(arg_vals.size())
          {
            case 0:
              return runtime::dynamic_call(source);
            case 1:
              return runtime::dynamic_call(source, arg_vals[0]);
            case 2:
              return runtime::dynamic_call(source, arg_vals[0], arg_vals[1]);
            case 3:
              return runtime::dynamic_call(source, arg_vals[0], arg_vals[1], arg_vals[2]);
            case 4:
              return runtime::dynamic_call(source,
                                           arg_vals[0],
                                           arg_vals[1],
                                           arg_vals[2],
                                           arg_vals[3]);
            case 5:
              return runtime::dynamic_call(source,
                                           arg_vals[0],
                                           arg_vals[1],
                                           arg_vals[2],
                                           arg_vals[3],
                                           arg_vals[4]);
            case 6:
              return runtime::dynamic_call(source,
                                           arg_vals[0],
                                           arg_vals[1],
                                           arg_vals[2],
                                           arg_vals[3],
                                           arg_vals[4],
                                           arg_vals[5]);
            case 7:
              return runtime::dynamic_call(source,
                                           arg_vals[0],
                                           arg_vals[1],
                                           arg_vals[2],
                                           arg_vals[3],
                                           arg_vals[4],
                                           arg_vals[5],
                                           arg_vals[6]);
            case 8:
              return runtime::dynamic_call(source,
                                           arg_vals[0],
                                           arg_vals[1],
                                           arg_vals[2],
                                           arg_vals[3],
                                           arg_vals[4],
                                           arg_vals[5],
                                           arg_vals[6],
                                           arg_vals[7]);
            case 9:
              return runtime::dynamic_call(source,
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
              return runtime::dynamic_call(source,
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
                /* TODO: This could be optimized; making lists sucks right now. */
                runtime::detail::native_persistent_list all{ arg_vals.rbegin(), arg_vals.rend() };
                for(size_t i{}; i < 10; ++i)
                {
                  all = all.rest();
                }
                return runtime::dynamic_call(source,
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
                                             make_box<runtime::obj::persistent_list>(all));
              }
          }
        }
        else if constexpr(std::same_as<T, runtime::obj::persistent_set>)
        {
          auto const s(expr.arg_exprs.size());
          if(s != 1)
          {
            throw std::runtime_error{
              fmt::format("invalid call with {} args to: {}", s, typed_source->to_string())
            };
          }
          return typed_source->call(eval(rt_ctx, jit_prc, expr.arg_exprs[0]));
        }
        else if constexpr(std::same_as<T, runtime::obj::keyword>
                          || std::same_as<T, runtime::obj::persistent_hash_map>
                          || std::same_as<T, runtime::obj::persistent_array_map>)
        {
          auto const s(expr.arg_exprs.size());
          switch(s)
          {
            case 1:
              return typed_source->call(eval(rt_ctx, jit_prc, expr.arg_exprs[0]));
            case 2:
              return typed_source->call(eval(rt_ctx, jit_prc, expr.arg_exprs[0]),
                                        eval(rt_ctx, jit_prc, expr.arg_exprs[1]));
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

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &,
                           analyze::expr::primitive_literal<analyze::expression> const &expr)
  {
    if(expr.data->type == runtime::object_type::keyword)
    {
      auto const d(runtime::expect_object<runtime::obj::keyword>(expr.data));
      return rt_ctx.intern_keyword(d->sym, true).expect_ok();
    }
    return expr.data;
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::vector<analyze::expression> const &expr)
  {
    runtime::detail::native_transient_vector ret;
    for(auto const &e : expr.data_exprs)
    {
      ret.push_back(eval(rt_ctx, jit_prc, e));
    }
    if(expr.meta.is_some())
    {
      return make_box<runtime::obj::persistent_vector>(expr.meta.unwrap(), ret.persistent());
    }
    else
    {
      return make_box<runtime::obj::persistent_vector>(ret.persistent());
    }
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::map<analyze::expression> const &expr)
  {
    auto const size(expr.data_exprs.size());
    if(size <= runtime::obj::persistent_array_map::max_size)
    {
      auto const array_box(make_array_box<runtime::object_ptr>(size * 2));
      size_t i{};
      for(auto const &e : expr.data_exprs)
      {
        array_box.data[i++] = eval(rt_ctx, jit_prc, e.first);
        array_box.data[i++] = eval(rt_ctx, jit_prc, e.second);
      }

      if(expr.meta.is_some())
      {
        return make_box<runtime::obj::persistent_array_map>(expr.meta.unwrap(),
                                                            runtime::detail::in_place_unique{},
                                                            array_box,
                                                            size * 2);
      }
      else
      {
        return make_box<runtime::obj::persistent_array_map>(runtime::detail::in_place_unique{},
                                                            array_box,
                                                            size * 2);
      }
    }
    else
    {
      runtime::detail::native_transient_hash_map trans;
      for(auto const &e : expr.data_exprs)
      {
        trans.insert({ eval(rt_ctx, jit_prc, e.first), eval(rt_ctx, jit_prc, e.second) });
      }

      if(expr.meta.is_some())
      {
        return make_box<runtime::obj::persistent_hash_map>(expr.meta.unwrap(), trans.persistent());
      }
      else
      {
        return make_box<runtime::obj::persistent_hash_map>(trans.persistent());
      }
    }
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::set<analyze::expression> const &expr)
  {
    runtime::detail::native_transient_set ret;
    for(auto const &e : expr.data_exprs)
    {
      ret.insert(eval(rt_ctx, jit_prc, e));
    }
    if(expr.meta.is_some())
    {
      return make_box<runtime::obj::persistent_set>(expr.meta.unwrap(), std::move(ret));
    }
    else
    {
      return make_box<runtime::obj::persistent_set>(std::move(ret));
    }
  }

  runtime::object_ptr
  eval(runtime::context &, jit::processor const &, analyze::expr::local_reference const &)
  /* Doesn't make sense to eval these, since let is wrapped in a fn and JIT compiled. */
  {
    throw make_box("unsupported eval: local_reference");
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::function<analyze::expression> const &expr)
  {
    auto const &module(runtime::module::nest_module(
      runtime::expect_object<runtime::ns>(rt_ctx.current_ns_var->deref())->to_string(),
      runtime::munge(expr.name)));
    codegen::processor cg_prc{ rt_ctx, expr, module, codegen::compilation_target::repl };
    //cg_prc.gen_body();
    //cg_prc.jit_module->print(llvm::outs(), nullptr);
    return jit_prc.eval(cg_prc).expect_ok().unwrap();
    //return runtime::obj::nil::nil_const();
  }

  runtime::object_ptr eval(runtime::context &,
                           jit::processor const &,
                           analyze::expr::recur<analyze::expression> const &)
  /* Recur will always be in a fn or loop, which will be JIT compiled. */
  {
    throw make_box("unsupported eval: recur");
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::do_<analyze::expression> const &expr)
  {
    runtime::object_ptr ret{ runtime::obj::nil::nil_const() };
    for(auto const &form : expr.body)
    {
      ret = eval(rt_ctx, jit_prc, form);
    }
    return ret;
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::let<analyze::expression> const &expr)
  {
    return runtime::dynamic_call(eval(rt_ctx, jit_prc, wrap_expression(expr)));
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::if_<analyze::expression> const &expr)
  {
    auto const condition(eval(rt_ctx, jit_prc, expr.condition));
    if(runtime::detail::truthy(condition))
    {
      return eval(rt_ctx, jit_prc, expr.then);
    }
    else if(expr.else_.is_some())
    {
      return eval(rt_ctx, jit_prc, expr.else_.unwrap());
    }
    return runtime::obj::nil::nil_const();
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::throw_<analyze::expression> const &expr)
  {
    throw eval(rt_ctx, jit_prc, expr.value);
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::try_<analyze::expression> const &expr)
  {
    return runtime::dynamic_call(eval(rt_ctx, jit_prc, wrap_expression(expr)));
  }

  runtime::object_ptr eval(runtime::context &rt_ctx,
                           jit::processor const &jit_prc,
                           analyze::expr::native_raw<analyze::expression> const &expr)
  {
    return runtime::dynamic_call(eval(rt_ctx, jit_prc, wrap_expression(expr)));
  }
}
