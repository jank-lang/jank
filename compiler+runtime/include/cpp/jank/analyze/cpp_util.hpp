#pragma once

#include <clang/Interpreter/CppInterOp.h>

#include <jtl/ptr.hpp>
#include <jtl/result.hpp>

#include <jank/analyze/expression.hpp>
#include <jank/error.hpp>

namespace jank::analyze::cpp_util
{
  struct literal_value_result
  {
    jtl::ptr<void> fn_scope{};
    jtl::ptr<void> ret_type{};
    jtl::immutable_string function_code{};
  };

  jtl::string_result<void> instantiate_if_needed(jtl::ptr<void> const scope);

  jtl::ptr<void> apply_pointers(jtl::ptr<void> type, u8 ptr_count);
  jtl::ptr<void> resolve_type(jtl::immutable_string const &sym, u8 ptr_count);
  jtl::string_result<jtl::ptr<void>> resolve_scope(jtl::immutable_string const &sym);
  jtl::string_result<jtl::ptr<void>> resolve_literal_type(jtl::immutable_string const &literal);
  jtl::string_result<literal_value_result>
  resolve_literal_value(jtl::immutable_string const &literal);
  native_vector<jtl::ptr<void>> find_adl_scopes(native_vector<jtl::ptr<void>> const &starters);

  jtl::immutable_string get_qualified_name(jtl::ptr<void> scope);

  jtl::ptr<void> expression_type(expression_ref expr);
  jtl::ptr<void> non_void_expression_type(expression_ref expr);

  jtl::string_result<std::vector<Cpp::TemplateArgInfo>>
  find_best_arg_types_with_conversions(std::vector<void *> const &fns,
                                       std::vector<Cpp::TemplateArgInfo> const &args,
                                       bool is_member_call);
  jtl::string_result<jtl::ptr<void>>
  find_best_overload(std::vector<void *> const &fns, std::vector<Cpp::TemplateArgInfo> const &args);

  bool is_trait_convertible(jtl::ptr<void> type);
  bool is_untyped_object(jtl::ptr<void> type);
  bool is_typed_object(jtl::ptr<void> type);
  bool is_any_object(jtl::ptr<void> type);
  bool is_primitive(jtl::ptr<void> type);
  bool is_member_function(jtl::ptr<void> scope);
  bool is_non_static_member_function(jtl::ptr<void> scope);
  bool is_nullptr(jtl::ptr<void> type);
  bool is_implicitly_convertible(jtl::ptr<void> from, jtl::ptr<void> to);

  jtl::ptr<void> untyped_object_ptr_type();
  jtl::ptr<void> untyped_object_ref_type();

  usize offset_to_typed_object_base(jtl::ptr<void> type);

  jtl::option<Cpp::Operator> match_operator(jtl::immutable_string const &name);
  jtl::option<jtl::immutable_string> operator_name(Cpp::Operator const op);

  jtl::result<void, error_ref> ensure_convertible(expression_ref const expr);
}
