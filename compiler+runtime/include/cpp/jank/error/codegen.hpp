#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref internal_codegen_failure(jtl::immutable_string const &message);
}
