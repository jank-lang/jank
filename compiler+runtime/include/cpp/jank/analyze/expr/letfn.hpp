#pragma once

#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/expr/function.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using letfn_ref = jtl::ref<struct letfn>;

  struct letfn : expression
  {
    using pair_type = std::pair<runtime::obj::symbol_ref, function_ref>;

    static constexpr expression_kind expr_kind{ expression_kind::letfn };

    letfn(expression_position position, local_frame_ptr frame, native_bool needs_box, do_ref body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    native_vector<pair_type> pairs;
    do_ref body;
  };
}
