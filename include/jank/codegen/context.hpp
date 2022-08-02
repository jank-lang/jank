#pragma once

#include <sstream>

#include <jank/analyze/expression.hpp>

namespace jank::runtime
{ struct context; }

namespace jank::codegen
{
  /* There's only one codegen context per thread, so no synchronization is needed for its members. */
  struct context
  {
    context() = delete;
    context(runtime::context &ctx);
    context(context const &) = delete;
    context(context &&) = default;

    void gen(analyze::expression const &);
    void gen(analyze::expr::def<analyze::expression> const &);
    void gen(analyze::expr::var_deref<analyze::expression> const &);
    void gen(analyze::expr::call<analyze::expression> const &);
    void gen(analyze::expr::primitive_literal<analyze::expression> const &);
    void gen(analyze::expr::vector<analyze::expression> const &);
    void gen(analyze::expr::local_reference<analyze::expression> const &);
    void gen(analyze::expr::function<analyze::expression> const &);

    /* TODO: C++20: Return std::string_view. */
    std::string data() const;

    runtime::context &runtime_ctx;
    std::ostringstream oss;
  };
}
