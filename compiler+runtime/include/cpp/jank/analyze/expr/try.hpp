#pragma once

#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
}

namespace jank::analyze::expr
{
  using do_ptr = runtime::native_box<struct do_>;

  struct catch_
  {
    void propagate_position(expression_position const pos);
    runtime::object_ptr to_runtime_data() const;

    runtime::obj::symbol_ptr sym{};
    do_ptr body{};
  };

  using try_ptr = runtime::native_box<struct try_>;

  struct try_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::try_ };

    try_(expression_position position, local_frame_ptr frame, native_bool needs_box, do_ptr body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ptr to_runtime_data() const override;

    do_ptr body{};
    option<catch_> catch_body{};
    option<do_ptr> finally_body{};
  };
}
