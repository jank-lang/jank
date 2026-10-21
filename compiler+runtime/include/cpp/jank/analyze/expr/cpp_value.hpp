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
      bool_true,
      bool_false,
      constructor,
      variable,
      enum_constant,
      function,

      /* The following must be in call position. */
      member_access,
      member_call,
      operator_call
    };

    static constexpr char const *value_kind_str(value_kind const kind)
    {
      switch(kind)
      {
        case value_kind::null:
          return "null";
        case value_kind::bool_true:
          return "bool_true";
        case value_kind::bool_false:
          return "bool_false";
        case value_kind::constructor:
          return "constructor";
        case value_kind::variable:
          return "variable";
        case value_kind::enum_constant:
          return "enum_constant";
        case value_kind::function:
          return "function";
        case value_kind::member_access:
          return "member_access";
        case value_kind::member_call:
          return "member_call";
        case value_kind::operator_call:
          return "operator_call";
        default:
          return "unknown";
      }
    }

    cpp_value(expression_position position,
              local_frame_ptr frame,
              bool needs_box,
              runtime::obj::symbol_ref const sym,
              jtl::ptr<void> type,
              jtl::ptr<void> scope,
              value_kind val_kind);

    cpp_value(expression_position position,
              local_frame_ptr frame,
              bool needs_box,
              runtime::object_ref form,
              runtime::obj::symbol_ref const sym,
              jtl::ptr<void> type,
              jtl::ptr<void> scope,
              value_kind val_kind);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    jtl::ptr<void> get_type() const override;

    /* The original value form of either the type symbol or the value
     * being casted. We use this mainly for source info. */
    /* TODO: Just store source info! We don't always have a symbol. */
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
