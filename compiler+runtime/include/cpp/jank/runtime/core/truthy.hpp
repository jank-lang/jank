#pragma once

#include <jank/runtime/oref.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  template <typename T>
  bool truthy(T const d)
  {
    if constexpr(std::same_as<T, object_ref>)
    {
      if(d.is_nil())
      {
        return false;
      }

      if(detail::is_tagged_small_int(d.raw()))
      {
        return true;
      }
      if(detail::is_small_real(d.raw()))
      {
        return true;
      }

      auto const b{ dyn_cast<obj::boolean>(d) };
      if(b.is_some())
      {
        return b->data;
      }

      return true;
    }
    if constexpr(std::same_as<T, obj::nil_ref>)
    {
      return false;
    }
    else if constexpr(std::same_as<T, obj::boolean_ref>)
    {
      return d.is_some() && d->data;
    }
    else if constexpr(std::same_as<T, bool>)
    {
      return d;
    }
    else
    {
      return true;
    }
  }
}
