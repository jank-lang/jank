#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref codegen_internal_failure(jtl::immutable_string const &message);
}
