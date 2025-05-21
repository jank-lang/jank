#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze
{
  enum class conversion_policy : u8
  {
    into_object,
    from_object
  };
}

namespace jank::analyze::expr
{
  using cpp_cast_ref = jtl::ref<struct cpp_cast>;

  /* Cast expressions are only used for conversion calls using jank's
   * conversion trait. During analysis, if the cast can be done using
   * normal construction, we'll create a cpp_constructor_call expression
   * instead. */
  /* TODO: Rename to cpp_conversion or something. */
  struct cpp_cast : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_cast };

    cpp_cast(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             jtl::ptr<void> type,
             jtl::ptr<void> conversion_type,
             conversion_policy policy,
             expression_ref value_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    /* This is the resulting type of the conversion. */
    jtl::ptr<void> type{};
    /* This type is used for our conversion trait. Depending on which type is
     * an object, we may need to cast to/from object. That's where the conversion
     * policy comes in. */
    jtl::ptr<void> conversion_type{};
    conversion_policy policy{};
    expression_ref value_expr;
  };
}
