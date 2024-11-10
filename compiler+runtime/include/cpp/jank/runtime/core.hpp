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

  object_ptr meta(object_ptr m);
  object_ptr with_meta(object_ptr o, object_ptr m);
  object_ptr reset_meta(object_ptr o, object_ptr m);

  obj::persistent_string_ptr subs(object_ptr s, object_ptr start);
  obj::persistent_string_ptr subs(object_ptr s, object_ptr start, object_ptr end);
  native_integer first_index_of(object_ptr s, object_ptr m);
  native_integer last_index_of(object_ptr s, object_ptr m);

  native_bool is_named(object_ptr o);
  native_persistent_string name(object_ptr o);
  native_persistent_string namespace_(object_ptr o);

  native_bool is_callable(object_ptr o);

  object_ptr force(object_ptr o);
}
