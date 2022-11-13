#pragma once

#include <sstream>

#include <folly/FBVector.h>

#include <fmt/format.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::runtime
{ struct context; }

namespace jank::codegen
{
  /* Codegen processors render a single function expression to a C++ functor. REPL expressions
   * are wrapped in a nullary functor. These functors nest arbitrarily, if an expression has more
   * fn values of its own, each one rendered with its own codegen processor. */
  struct processor
  {
    processor() = delete;
    processor
    (
      runtime::context &rt_ctx,
      analyze::context &an_ctx,
      analyze::expression const &expr
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
    void gen(analyze::expr::local_reference const &, bool const is_statement);
    void gen(analyze::expr::function<analyze::expression> const &, bool const is_statement);
    void gen(analyze::expr::native_raw<analyze::expression> const &, bool const is_statement);

    std::string declaration_str();
    void build_header();
    void build_body();
    void build_footer();
    std::string expression_str(bool const auto_call = true);

    runtime::context &rt_ctx;
    analyze::context &an_ctx;
    analyze::expr::function<analyze::expression> root_expression;

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
