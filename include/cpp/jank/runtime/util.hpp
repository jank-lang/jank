#pragma once

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr o);
    bool truthy(obj::nil_ptr);
    bool truthy(obj::boolean_ptr const o);
    bool truthy(native_bool const o);
  }
  native_string munge(native_string const &o);
  object_ptr munge(object_ptr o);
}
