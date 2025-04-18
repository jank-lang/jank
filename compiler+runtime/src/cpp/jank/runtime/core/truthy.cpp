#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  bool truthy(object const *o)
  {
    if(!o)
    {
      return false;
    }

    if(o->type == object_type::nil)
    {
      return false;
    }

    auto const b{ dyn_cast<obj::boolean>(o) };
    if(b.is_some())
    {
      return b->data;
    }

    return true;
  }

  bool truthy(object_ref const o)
  {
    return truthy(o.data);
  }

  bool truthy(obj::nil_ref)
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
