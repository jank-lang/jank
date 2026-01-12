#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  bool equal(char lhs, object_ref const rhs);
  bool equal(object_ref const lhs, object_ref const rhs);
  i64 compare(object_ref const, object_ref const);
  bool is_identical(object_ref const lhs, object_ref const rhs);
}
