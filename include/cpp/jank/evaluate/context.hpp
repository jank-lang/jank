#pragma once

#include <jank/runtime/object.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/evaluate/frame.hpp>

namespace jank::runtime
{ struct context; }

/* TODO: Remove this whole thing? */
/* TODO: Rename this ns to interpret. */
namespace jank::evaluate
{
  /* There's only one eval context per thread, so no synchronization is needed for its members. */
  struct context
  {
    context() = delete;
    context(runtime::context &ctx);
    context(context const &) = default;
    context(context &&) noexcept = default;

    runtime::object_ptr eval(analyze::expression const &);
    runtime::object_ptr eval(analyze::expression const &, frame const &);
    runtime::object_ptr eval(analyze::expr::def<analyze::expression> const &, frame const &);
    runtime::object_ptr eval(analyze::expr::var_deref<analyze::expression> const &, frame const &);
    runtime::object_ptr eval(analyze::expr::call<analyze::expression> const &, frame const &);
    runtime::object_ptr eval(analyze::expr::primitive_literal<analyze::expression> const &, frame const &);
    runtime::object_ptr eval(analyze::expr::vector<analyze::expression> const &, frame const &);
    runtime::object_ptr eval(analyze::expr::local_reference<analyze::expression> const &, frame const &);
    runtime::object_ptr eval(analyze::expr::function<analyze::expression> const &, frame const &);

    runtime::object_ptr interpret(analyze::expr::function<analyze::expression> const &expr, frame const &current_frame);

    runtime::context &runtime_ctx;
    frame root_frame;
  };
}
