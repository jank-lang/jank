#pragma once

#include <sstream>

#include <folly/FBVector.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::runtime
{ struct context; }

namespace jank::codegen
{
  /* There's only one codegen context per thread, so no synchronization is needed for its members. */
  struct context
  {
    context() = delete;
    context
    (
      runtime::context &rt_ctx,
      analyze::context &an_ctx,
      analyze::processor::iterator const an_begin,
      analyze::processor::iterator const an_end,
      size_t const total_forms
    );
    context(context const &) = delete;
    context(context &&) = default;

    void gen(analyze::expression const &, std::ostream &os) const;
    void gen(analyze::expr::def<analyze::expression> const &, std::ostream &os) const;
    void gen(analyze::expr::var_deref<analyze::expression> const &, std::ostream &os) const;
    void gen(analyze::expr::call<analyze::expression> const &, std::ostream &os) const;
    void gen(analyze::expr::primitive_literal<analyze::expression> const &, std::ostream &os) const;
    void gen(analyze::expr::vector<analyze::expression> const &, std::ostream &os) const;
    void gen(analyze::expr::local_reference<analyze::expression> const &, std::ostream &os) const;
    void gen(analyze::expr::function<analyze::expression> const &, std::ostream &os) const;

    /* TODO: C++20: Return std::string_view. */
    std::string str() const;
    void header_str(std::ostream &) const;
    void body_str(std::ostream &) const;
    void footer_str(std::ostream &) const;

    runtime::context &rt_ctx;
    analyze::context &an_ctx;
    folly::fbvector<analyze::expression> expressions;
    mutable runtime::obj::symbol struct_name;
    mutable bool need_semi_colon{ true };
  };
}
