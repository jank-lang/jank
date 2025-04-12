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

  jtl::ptr<void> find_best_overload_with_conversions(std::vector<void *> const &fns,
                                                     std::vector<Cpp::TemplateArgInfo> const &args);
}
