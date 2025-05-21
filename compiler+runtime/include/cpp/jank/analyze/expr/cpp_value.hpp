#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using cpp_value_ref = jtl::ref<struct cpp_value>;

  struct cpp_value : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_value };

    enum class value_kind : u8
    {
      null,
      constructor,
      variable,
      enum_constant,
      function,
      member_access,
      member_call
    };

    cpp_value(expression_position position,
              local_frame_ptr frame,
              bool needs_box,
              runtime::obj::symbol_ref sym,
              jtl::ptr<void> type,
              jtl::ptr<void> scope,
              value_kind val_kind);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    /* The original value form of either the type symbol or the value
     * being casted. We use this mainly for source info. */
    runtime::object_ref form;
    /* In general, this represents the type of the value. For constructors,
     * this will represent the type we're constructing. */
    jtl::ptr<void> type;
    /* If the type is primitive, the scope will be null. Built-in types don't have
     * scopes in Clang. */
    jtl::ptr<void> scope;
    value_kind val_kind;
  };
}
