#pragma once

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/math.hpp>

namespace jank::runtime
{
  /* TODO: Header for this, with sequence equality fns. */
  native_bool equal(object_ptr lhs, object_ptr rhs);
  native_integer compare(object_ptr, object_ptr);
}
