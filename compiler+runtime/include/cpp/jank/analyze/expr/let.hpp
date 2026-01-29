#pragma once

#include <jank/analyze/expr/do.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using let_ref = jtl::ref<struct let>;

  struct let : expression
  {
    using pair_type = std::pair<local_binding_ref, expression_ref>;

    static constexpr expression_kind expr_kind{ expression_kind::let };

    let(expression_position position, local_frame_ptr frame, bool needs_box, do_ref body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    native_vector<pair_type> pairs;
    do_ref body;
    /* let* and loop* share the same expression (Clojure JVM does the same). In fact,
     * a loop* is only considered a loop if it also contains a recur. Otherwise, it's
     * just a let*. */
    bool is_loop{};
  };
}
