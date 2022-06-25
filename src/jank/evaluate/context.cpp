#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/evaluate/context.hpp>

namespace jank::evaluate
{
  context::context(runtime::context &ctx)
    : runtime_ctx{ ctx }
  { }

  runtime::object_ptr context::eval(analyze::expression const &ex)
  {
    runtime::object_ptr ret;
    boost::apply_visitor
    (
      [this, &ret](auto const &typed_ex)
      { ret = eval(typed_ex); },
      ex.data
    );
    return ret;
  }

  runtime::object_ptr context::eval(analyze::expr::def<analyze::expression> const &expr)
  {
    auto &t_state(runtime_ctx.get_thread_state(none));
    auto const &ns_ptr(boost::static_pointer_cast<runtime::ns>(t_state.current_ns->get_root()));
    auto locked_vars(ns_ptr->vars.ulock());
    auto const &existing(locked_vars->find(expr.name));
    if(existing != locked_vars->end())
    {
      existing->second->set_root(expr.value);
      return existing->second;
    }
    else
    {
      auto var(runtime::var::create(ns_ptr, expr.name, expr.value));
      auto write_locked_vars(locked_vars.moveFromUpgradeToWrite());
      auto const &res(write_locked_vars->insert({expr.name, std::move(var)}));
      return res.first->second;
    }
  }

  runtime::object_ptr context::eval(analyze::expr::var_deref<analyze::expression> const &expr)
  { return expr.var->get_root(); }

  runtime::object_ptr context::eval(analyze::expr::call<analyze::expression> const &expr)
  {
    switch(expr.arg_exprs.size())
    {
      case 0:
        return expr.fn->call();
      case 1:
        return expr.fn->call(expr.args->data.first().unwrap());
      case 2:
        return expr.fn->call(expr.args->data.first().unwrap(), expr.args->data.rest().first().unwrap());
      default:
        throw "unsupported arg count";
    }
  }

  runtime::object_ptr context::eval(analyze::expr::literal<analyze::expression> const &expr)
  { return expr.data; }

  runtime::object_ptr context::eval(analyze::expr::local_reference<analyze::expression> const &expr)
  { return expr.name; }

  runtime::object_ptr context::eval(analyze::expr::function<analyze::expression> const &)
  { return {}; }
}
