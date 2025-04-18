#pragma once

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/nil.hpp>

namespace jank::runtime
{
  bool truthy(object const *o);
  bool truthy(object_ref o);
  bool truthy(obj::nil_ref);
  bool truthy(obj::boolean_ref const o);
  bool truthy(bool const o);

  template <typename T>
  requires runtime::behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto truthy(T const * const d)
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

  template <typename T>
  requires runtime::behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto truthy(oref<T> const &d)
  {
    return truthy(d.erase());
  }
}
