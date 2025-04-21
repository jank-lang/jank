#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  bool equal(char lhs, object_ref rhs);
  bool equal(object_ref lhs, object_ref rhs);
  i64 compare(object_ref, object_ref);
  bool is_identical(object_ref lhs, object_ref rhs);
}
