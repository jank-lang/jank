#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  bool truthy(object_ref const o)
  {
    if(o.is_nil())
    {
      return false;
    }

    if(detail::is_small_int(o.data))
    {
      return true;
    }

    auto const b{ dyn_cast<obj::boolean>(o) };
    if(b.is_some())
    {
      return b->data;
    }

    return true;
  }

  bool truthy(obj::nil_ref const)
  {
    return false;
  }

  bool truthy(obj::boolean_ref const o)
  {
    return o.is_some() && o->data;
  }

  bool truthy(bool const o)
  {
    return o;
  }
}
