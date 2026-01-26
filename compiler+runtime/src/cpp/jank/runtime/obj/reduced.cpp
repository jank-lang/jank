#include <jank/runtime/obj/reduced.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  reduced::reduced(object_ref const o)
    : object{ obj_type, obj_behaviors }
    , val{ o }
  {
    jank_debug_assert(val.is_some());
  }

  object_ref reduced::deref() const
  {
    return val;
  }
}
