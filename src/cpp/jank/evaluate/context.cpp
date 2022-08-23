#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/evaluate/context.hpp>

namespace jank::evaluate
{
  context::context(runtime::context &ctx)
    : runtime_ctx{ ctx }
  { }

  runtime::object_ptr context::eval(analyze::expression const &ex)
  { return eval(ex, root_frame); }
  runtime::object_ptr context::eval(analyze::expression const &ex, frame const &current_frame)
  {
    runtime::object_ptr ret;
    boost::apply_visitor
    (
      [this, &ret, &current_frame](auto const &typed_ex)
      { ret = eval(typed_ex, current_frame); },
      ex.data
    );
    return ret;
  }

  runtime::object_ptr context::eval(analyze::expr::def<analyze::expression> const &expr, frame const &current_frame)
  {
    auto &t_state(runtime_ctx.get_thread_state(none));
    auto const &ns_ptr(boost::static_pointer_cast<runtime::ns>(t_state.current_ns->get_root()));
    auto const evaluated_value(eval(*expr.value, current_frame));

    auto locked_vars(ns_ptr->vars.ulock());
    auto const &existing(locked_vars->find(expr.name));
    if(existing != locked_vars->end())
    {
      existing->second->set_root(evaluated_value);
      return existing->second;
    }
    else
    {
      auto var(runtime::var::create(ns_ptr, expr.name, evaluated_value));
      auto write_locked_vars(locked_vars.moveFromUpgradeToWrite());
      auto const &res(write_locked_vars->insert({expr.name, std::move(var)}));
      return res.first->second;
    }
  }

  runtime::object_ptr context::eval(analyze::expr::var_deref<analyze::expression> const &expr, frame const &)
  {
    auto const var(runtime_ctx.find_var(expr.qualified_name));
    return var.unwrap()->get_root();
  }

  runtime::object_ptr context::eval(analyze::expr::call<analyze::expression> const &, frame const &)
  {
    return nullptr;
    //switch(expr.arg_exprs.size())
    //{
    //  case 0:
    //    return expr.fn->call();
    //  case 1:
    //    return expr.fn->call(expr.args->data.first().unwrap());
    //  case 2:
    //    return expr.fn->call(expr.args->data.first().unwrap(), expr.args->data.rest().first().unwrap());
    //  default:
    //    throw "unsupported arg count";
    //}
  }

  runtime::object_ptr context::eval(analyze::expr::primitive_literal<analyze::expression> const &expr, frame const &)
  { return expr.data; }

  runtime::object_ptr context::eval(analyze::expr::vector<analyze::expression> const &expr, frame const &current_frame)
  {
    runtime::detail::vector_transient_type ret;
    for(auto const &e : expr.data_exprs)
    { ret.push_back(eval(e, current_frame)); }
    return runtime::obj::vector::create(ret.persistent());
  }

  runtime::object_ptr context::eval(analyze::expr::local_reference<analyze::expression> const &expr, frame const &current_frame)
  { return current_frame.find(expr.name).unwrap().value; }

  runtime::object_ptr context::interpret(analyze::expr::function<analyze::expression> const &expr, frame const &current_frame)
  {
    runtime::object_ptr ret;
    for(auto const &body_expr : expr.body.body)
    { ret = eval(body_expr, current_frame); }
    return ret;
  }

  runtime::object_ptr context::eval(analyze::expr::function<analyze::expression> const &expr, frame const &current_frame)
  {
    /* TODO: JIT fn bodies, don't interpret them. */
    switch(expr.params.size())
    {
      case 0:
        return runtime::obj::function::create
        (
          std::function<runtime::object_ptr ()>
          {
            [this, current_frame, expr]()
            { return interpret(expr, current_frame); }
          }
        );
      case 1:
        return runtime::obj::function::create
        (
          std::function<runtime::object_ptr (runtime::object_ptr const&)>
          {
            [this, current_frame, expr](runtime::object_ptr const &a0)
            {
              frame fn_frame{ some(std::ref(current_frame)) };
              fn_frame.locals[expr.params[0]] = { expr.params[0], a0 };
              return interpret(expr, fn_frame);
            }
          }
        );
      case 2:
        return runtime::obj::function::create
        (
          std::function<runtime::object_ptr (runtime::object_ptr const&, runtime::object_ptr const&)>
          {
            [this, current_frame, expr](runtime::object_ptr const &a0, runtime::object_ptr const &a1)
            {
              frame fn_frame{ some(std::ref(current_frame)) };
              fn_frame.locals[expr.params[0]] = { expr.params[0], a0 };
              fn_frame.locals[expr.params[1]] = { expr.params[1], a1 };
              return interpret(expr, fn_frame);
            }
          }
        );
      default:
        throw "unsupported arg count";
    }
  }
}
