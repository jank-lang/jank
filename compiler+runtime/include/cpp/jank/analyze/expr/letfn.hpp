#pragma once

#include <jank/analyze/expr/do.hpp>

//#include <jank/analyze/expr/function.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
}

namespace jank::analyze::expr
{
  using letfn_ptr = runtime::native_box<struct letfn>;

  struct letfn : expression
  {
    // TODO function_ptr vals
    using pair_type = std::pair<runtime::obj::symbol_ptr, expression_ptr>;

    static constexpr expression_kind expr_kind{ expression_kind::letfn };

    letfn(expression_position position, local_frame_ptr frame, native_bool needs_box, do_ptr body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ptr to_runtime_data() const override;

    native_vector<pair_type> pairs;
    do_ptr body{};
  };
}
