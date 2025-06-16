#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_string_ref = oref<struct persistent_string>;
}

namespace jank::analyze::expr
{
  using cpp_raw_ref = jtl::ref<struct cpp_raw>;

  struct cpp_raw : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_raw };

    cpp_raw(expression_position position,
            local_frame_ptr frame,
            bool needs_box,
            jtl::immutable_string const &code);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    jtl::immutable_string code;
  };
}
