#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref aot_compilation_failure();
  error_ref aot_clang_executable_not_found();
  error_ref aot_module_not_found(jtl::immutable_string module_name);
}
