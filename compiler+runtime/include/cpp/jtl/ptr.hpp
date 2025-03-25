#pragma once

#include <jtl/primitive.hpp>
#include <jtl/trait/transform.hpp>
#include <jtl/assert.hpp>

namespace jtl
{
  template <typename T>
  struct ptr
  {
    using value_type = T;

    constexpr ptr() = default;

    constexpr ptr(nullptr_t)
    {
    }

    constexpr ptr(remove_const_t<value_type> * const data)
      : data{ data }
    {
    }

    constexpr ptr(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
    }

    template <typename C>
    requires is_convertible<C *, T *>
    constexpr ptr(ptr<C> const data)
      : data{ data.data }
    {
    }

    constexpr value_type *operator->() const
    {
      jank_debug_assert(data);
      return data;
    }

    constexpr bool operator!() const
    {
      return !data;
    }

    constexpr value_type &operator*() const
    {
      jank_debug_assert(data);
      return *data;
    }

    constexpr bool operator==(nullptr_t) const
    {
      return data == nullptr;
    }

    constexpr bool operator==(ptr const &rhs) const
    {
      return data == rhs.data;
    }

    constexpr bool operator!=(nullptr_t) const
    {
      return data != nullptr;
    }

    constexpr bool operator!=(ptr const &rhs) const
    {
      return data != rhs.data;
    }

    constexpr bool operator<(ptr const &rhs) const
    {
      return data < rhs.data;
    }

    constexpr operator ptr<value_type const>() const
    {
      return data;
    }

    constexpr operator value_type *() const
    {
      return data;
    }

    //operator object *() const
    //{
    //  return &data->base;
    //}

    constexpr explicit operator bool() const
    {
      return data;
    }

    value_type *data{};
  };
}
