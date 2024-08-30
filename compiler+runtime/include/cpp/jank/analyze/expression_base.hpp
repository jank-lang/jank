#pragma once

namespace jank::analyze
{
  using namespace jank::runtime;

  enum class expression_type
  {
    nested,
    statement,
    return_statement
  };

  inline native_bool is_statement(expression_type const expr_type)
  {
    return expr_type != expression_type::nested;
  }

  /* Common base class for every expression. */
  struct expression_base : gc
  {
    object_ptr to_runtime_data() const
    {
      return obj::persistent_array_map::create_unique(make_box("expr_type"),
                                                      make_box(magic_enum::enum_name(expr_type)),
                                                      make_box("needs_box"),
                                                      make_box(needs_box));
    }

    expression_type expr_type{};
    local_frame_ptr frame{};
    native_bool needs_box{ true };
  };

  using expression_base_ptr = native_box<expression_base>;
}
