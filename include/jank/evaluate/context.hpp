#pragma once

#include <jank/runtime/context.hpp>
#include <jank/runtime/object.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::evaluate
{
  struct context
  {
    context() = delete;
    context(runtime::context &ctx);
    context(context const &) = default;
    context(context &&) = default;

    runtime::object_ptr eval(analyze::expression const &);
    runtime::object_ptr eval(analyze::expr::def<analyze::expression> const &);
    runtime::object_ptr eval(analyze::expr::var_deref<analyze::expression> const &);
    runtime::object_ptr eval(analyze::expr::call<analyze::expression> const &);
    runtime::object_ptr eval(analyze::expr::list<analyze::expression> const &);

    runtime::context &runtime_ctx;
  };
}
