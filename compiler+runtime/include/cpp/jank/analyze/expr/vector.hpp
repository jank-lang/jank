#pragma once

#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::analyze::expr
{
  using vector_ptr = runtime::native_box<struct vector>;

  struct vector : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::vector };

    vector(expression_position position,
           local_frame_ptr frame,
           native_bool needs_box,
           native_vector<expression_ptr> &&data_exprs,
           option<runtime::object_ptr> const &meta);

    runtime::object_ptr to_runtime_data() const override;

    native_vector<expression_ptr> data_exprs;
    option<runtime::object_ptr> meta;
  };
}
