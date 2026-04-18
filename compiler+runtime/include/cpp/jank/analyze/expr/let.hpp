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

    /* `let*` and `loop*` are the same, semantically, except that loops act as a `recur` target.
     * However, since loop bindings are mutable, we will analyze them differently. In order to
     * ensure loops without recurs don't just get treated as lets, we need to disambiguate. */
    enum class loop_kind : u8
    {
      /* A let. */
      none,
      /* A loop with one or more recur. */
      loop_with_recur,
      /* A loop with no recurs. Similar to a let, but the bindings are still type-erased. */
      loop_without_recur
    };

    let(expression_position position, local_frame_ptr frame, bool needs_box, do_ref body);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;
    jtl::ptr<void> get_type() const override;

    native_vector<pair_type> pairs;
    do_ref body;
    loop_kind loop_kind{ loop_kind::none };
  };
}
