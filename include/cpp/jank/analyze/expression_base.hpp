#pragma once

namespace jank::analyze
{
  enum class expression_type
  {
    expression,
    statement,
    return_statement
  };

  inline bool is_statement(expression_type const expr_type)
  { return expr_type != expression_type::expression; }

  /* Common base class for every expression. */
  struct expression_base : gc
  {
    /* TODO: Add frame here. */
    expression_type expr_type{};
    bool needs_box{ true };
  };
  /* TODO: native_box */
  using expression_base_ptr = expression_base*;
}
