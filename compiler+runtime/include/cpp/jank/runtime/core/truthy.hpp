#pragma once

#include <jank/runtime/oref.hpp>
#include <jank/runtime/obj/nil.hpp>

namespace jank::runtime
{
  bool truthy(object_ref const o);
  bool truthy(obj::nil_ref const);
  bool truthy(obj::boolean_ref const o);
  bool truthy(bool const o);

  template <typename T>
  requires runtime::behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto truthy(oref<T> const &d)
  {
    if constexpr(std::same_as<T, obj::nil>)
    {
      return false;
    }
    else if constexpr(std::same_as<T, obj::boolean>)
    {
      return d->data;
    }
    else
    {
      return true;
    }
  }
}
