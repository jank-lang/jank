#pragma once

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/nil.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr o);
    bool truthy(obj::nil_ptr);
    bool truthy(obj::boolean_ptr const o);
    bool truthy(native_bool const o);

    template <typename T>
    requires runtime::behavior::objectable<T>
    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    inline auto truthy(T const * const d)
    {
      if constexpr(std::same_as<T, obj::nil>)
      { return false; }
      else if constexpr(std::same_as<T, obj::boolean>)
      { return d->data; }
      else
      { return true; }
    }

    template <typename T>
    requires runtime::behavior::objectable<T>
    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    inline auto truthy(native_box<T> const &d)
    { return truthy(d.data); }
  }
  native_string munge(native_string const &o);
  object_ptr munge(object_ptr o);
}
