#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/number.hpp>
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
    auto var(runtime_ctx.intern_var(expr.name).expect_ok());
    if(expr.value.is_none())
    { return var; }

    auto const evaluated_value(eval(*expr.value.unwrap(), current_frame));
    var->set_root(evaluated_value);
    return var;
  }

  runtime::object_ptr context::eval(analyze::expr::var_deref<analyze::expression> const &expr, frame const &)
  {
    auto const var(runtime_ctx.find_var(expr.qualified_name));
    return var.unwrap()->get_root();
  }

  runtime::object_ptr context::eval(analyze::expr::call<analyze::expression> const &expr, frame const &current_frame)
  {
    auto const source(eval(expr.source, current_frame));
    auto const callable(source->as_callable());
    if(!callable)
    {
      /* TODO: Error handling. */
      std::cout << "unable to call: " << *source << std::endl;
      throw "call error";
    }

    std::vector<runtime::object_ptr> arg_vals;
    arg_vals.reserve(expr.arg_exprs.size());
    for(auto const &arg_expr: expr.arg_exprs)
    { arg_vals.emplace_back(eval(arg_expr, current_frame)); }

    switch(arg_vals.size())
    {
      case 0:
        return callable->call();
      case 1:
        return callable->call(arg_vals[0]);
      case 2:
        return callable->call(arg_vals[0], arg_vals[1]);
      default:
        throw "unsupported arg count";
    }
  }

  runtime::object_ptr context::eval(analyze::expr::primitive_literal<analyze::expression> const &expr, frame const &)
  {
    if(auto d = expr.data->as_keyword())
    { return runtime_ctx.intern_keyword(d->sym, d->resolved); }
    return expr.data;
  }

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
