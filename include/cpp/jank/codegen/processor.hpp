#pragma once

#include <sstream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <folly/FBVector.h>
#pragma clang diagnostic pop

#include <fmt/format.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::runtime
{ struct context; }

namespace jank::codegen
{
  /* There's only one codegen context per thread, so no synchronization is needed for its members. */
  struct processor
  {
    processor() = delete;
    processor
    (
      runtime::context &rt_ctx,
      analyze::context &an_ctx,
      analyze::processor::iterator const &an_begin,
      analyze::processor::iterator const &an_end,
      size_t const total_forms
    );
    processor(processor const &) = delete;
    processor(processor &&) noexcept = default;

    void gen(analyze::expression const &, bool const is_statement);
    void gen(analyze::expr::def<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::var_deref<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::call<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::primitive_literal<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::vector<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::map<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::local_reference<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::function<analyze::expression> const &, bool const is_statement);

    /* TODO: C++20: Return std::string_view. */
    std::string declaration_str();
    void build_header();
    void build_body();
    void build_footer();
    std::string expression_str();

    runtime::context &rt_ctx;
    analyze::context &an_ctx;
    folly::fbvector<analyze::expression> expressions;

    mutable runtime::obj::symbol struct_name;
    mutable bool need_semi_colon{ true };
    fmt::memory_buffer header_buffer;
    fmt::memory_buffer body_buffer;
    fmt::memory_buffer footer_buffer;
    fmt::memory_buffer expression_buffer;
    bool generated_declaration{};
    bool generated_expression{};
  };
}
