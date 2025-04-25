#pragma once

#include <clang/Interpreter/CppInterOp.h>

#include <jtl/ptr.hpp>
#include <jtl/result.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::cpp_util
{
  jtl::ptr<void> resolve_type(jtl::immutable_string const &sym);
  jtl::string_result<jtl::ptr<void>> resolve_scope(jtl::immutable_string const &sym);

  jtl::ptr<void> expression_type(expression_ref expr);

  jtl::string_result<std::vector<Cpp::TemplateArgInfo>>
  find_best_arg_types_with_conversions(std::vector<void *> const &fns,
                                       std::vector<Cpp::TemplateArgInfo> const &args);

  bool is_convertible(jtl::ptr<void> type);
  bool is_untyped_object(jtl::ptr<void> type);

  jtl::ptr<void> untyped_object_ptr_type();
  jtl::ptr<void> untyped_object_ref_type();
}
