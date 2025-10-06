#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref aot_unresolved_main(jtl::immutable_string const &message);
  error_ref aot_compilation_failure();
  error_ref aot_clang_executable_not_found();
  error_ref internal_aot_failure(jtl::immutable_string const &message);
}
