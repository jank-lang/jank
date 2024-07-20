#pragma once

#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime
{
  native_integer compare(object_ptr, object_ptr);

  native_bool equal(object_ptr lhs, object_ptr rhs);

  native_real to_real(object_ptr o);
}
