#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref system_clang_executable_not_found();
  error_ref system_failure(jtl::immutable_string const &message);
}
