#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  native_bool equal(char lhs, object_ptr rhs);
  native_bool equal(object_ptr lhs, object_ptr rhs);
  native_integer compare(object_ptr, object_ptr);
  native_bool is_identical(object_ptr lhs, object_ptr rhs);
}
