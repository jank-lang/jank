#include <jank/runtime/obj/volatile.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  volatile_::volatile_(object_ref const o)
    : object{ obj_type, obj_behaviors }
    , val{ o }
  {
    jank_debug_assert(val.is_some());
  }

  object_ref volatile_::deref() const
  {
    return val;
  }

  object_ref volatile_::reset(object_ref const o)
  {
    val = o;
    jank_debug_assert(val.is_some());
    return val;
  }
}
