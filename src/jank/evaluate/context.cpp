#include <iostream>

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
    auto const &ns_ptr(boost::static_pointer_cast<runtime::ns>(runtime_ctx.current_ns->root));
    auto const &existing(ns_ptr->vars.find(expr.name));
    if(existing != ns_ptr->vars.end())
    {
      existing->second->root = expr.value;
      return existing->second;
    }
    else
    {
      auto const &res(ns_ptr->vars.insert({expr.name, runtime::var::create(ns_ptr, expr.name, expr.value)}));
      return res.first->second;
    }
  }

  runtime::object_ptr context::eval(analyze::expr::var_deref<analyze::expression> const &expr)
  { return expr.var->root; }

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
}
