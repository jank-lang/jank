#include <jank/runtime/core/truthy.hpp>

namespace jank::runtime
{
  native_bool truthy(object_ptr const o)
  {
    if(!o)
    {
      return false;
    }

    return visit_object(
      [](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return false;
        }
        else if constexpr(std::same_as<T, obj::boolean>)
        {
          return typed_o->data;
        }
        else
        {
          return true;
        }
      },
      o);
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
