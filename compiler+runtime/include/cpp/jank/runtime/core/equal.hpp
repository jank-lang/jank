#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  native_bool equal(char lhs, object_ref rhs);
  native_bool equal(object_ref lhs, object_ref rhs);
  i64 compare(object_ref, object_ref);
  native_bool is_identical(object_ref lhs, object_ref rhs);
}
