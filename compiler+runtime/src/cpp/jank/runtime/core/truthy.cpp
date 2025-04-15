#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  native_bool truthy(object const *o)
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

  native_bool truthy(object_ref const o)
  {
    return truthy(o.data);
  }

  native_bool truthy(obj::nil_ref)
  {
    return false;
  }

  native_bool truthy(obj::boolean_ref const o)
  {
    return o.is_some() && o->data;
  }

  native_bool truthy(native_bool const o)
  {
    return o;
  }
}
