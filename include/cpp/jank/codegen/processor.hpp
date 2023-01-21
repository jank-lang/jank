#pragma once

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
      analyze::expression_ptr const &expr
    );
    processor
    (
      runtime::context &rt_ctx,
      analyze::expr::function<analyze::expression> const &expr
    );
    processor(processor const &) = delete;
    processor(processor &&) noexcept = default;

    option<native_string> gen
    (
      analyze::expression_ptr const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::def<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::var_deref<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::var_ref<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::call<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::primitive_literal<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::vector<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::map<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::local_reference const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::function<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::recur<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::let<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::do_<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::if_<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );
    option<native_string> gen
    (
      analyze::expr::native_raw<analyze::expression> const &,
      analyze::expr::function_arity<analyze::expression> const &
    );

    native_string declaration_str();
    void build_header();
    void build_body();
    void build_footer();
    native_string expression_str(bool const auto_call = true);

    runtime::context &rt_ctx;
    analyze::expression_ptr root_expr;
    analyze::expr::function<analyze::expression> const &root_fn;

    runtime::obj::symbol struct_name;
    fmt::memory_buffer header_buffer;
    fmt::memory_buffer body_buffer;
    fmt::memory_buffer footer_buffer;
    fmt::memory_buffer expression_buffer;
    bool generated_declaration{};
    bool generated_expression{};
  };
}
