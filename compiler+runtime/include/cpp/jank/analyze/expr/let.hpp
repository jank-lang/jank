#pragma once

#include <jank/analyze/expr/do.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
}

namespace jank::analyze::expr
{
  using let_ptr = runtime::native_box<struct let>;

  struct let : expression
  {
    using pair_type = std::pair<runtime::obj::symbol_ptr, expression_ptr>;

    static constexpr expression_kind expr_kind{ expression_kind::let };

    let(expression_position position, local_frame_ptr frame, native_bool needs_box, do_ptr body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ptr to_runtime_data() const override;

    native_vector<pair_type> pairs;
    do_ptr body{};
  };
}
