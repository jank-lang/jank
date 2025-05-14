#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using do_ref = jtl::ref<struct do_>;

  struct catch_
  {
    void propagate_position(expression_position const pos) const;
    runtime::object_ref to_runtime_data() const;

    runtime::obj::symbol_ref sym{};
    do_ref body;
  };

  using try_ref = jtl::ref<struct try_>;

  struct try_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::try_ };

    try_(expression_position position, local_frame_ptr frame, bool needs_box, do_ref body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    do_ref body;
    jtl::option<catch_> catch_body{};
    jtl::option<do_ref> finally_body{};
  };
}
