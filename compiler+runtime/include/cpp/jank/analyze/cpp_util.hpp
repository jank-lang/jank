#pragma once

#include <jtl/ptr.hpp>
#include <jtl/result.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::cpp_util
{
  jtl::ptr<void> resolve_type(jtl::immutable_string const &sym);
  jtl::string_result<jtl::ptr<void>> resolve_scope(jtl::immutable_string const &sym);
}
