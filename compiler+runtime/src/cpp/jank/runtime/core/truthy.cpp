#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/erasure.hpp>

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
    else if(auto const b = dyn_cast<obj::boolean>(o))
    {
      return b->data;
    }

    return true;
  }

  native_bool truthy(object_ptr const o)
  {
    return truthy(o.data);
  }

  native_bool truthy(obj::nil_ptr)
  {
    return false;
  }

  native_bool truthy(obj::boolean_ptr const o)
  {
    return o && o->data;
  }

  native_bool truthy(native_bool const o)
  {
    return o;
  }
}
