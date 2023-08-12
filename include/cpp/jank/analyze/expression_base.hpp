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
    expression_type expr_type{};
    local_frame_ptr frame{};
    native_bool needs_box{ true };
  };
  using expression_base_ptr = native_box<expression_base>;
}
