#pragma once

#include <jtl/primitive.hpp>
#include <jtl/trait/transform.hpp>
#include <jtl/assert.hpp>

namespace jtl
{
  template <typename T>
  struct ref
  {
    using value_type = T;

    constexpr ref() = delete;
    constexpr ref(nullptr_t) = delete;

    constexpr ref(remove_const_t<value_type> &data)
      : data{ &data }
    {
    }

    constexpr ref(value_type const &data)
      : data{ const_cast<value_type *>(&data) }
    {
    }

    template <typename C>
    requires is_convertible<C *, T *>
    constexpr ref(ref<C> const data)
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

    constexpr bool operator==(ref const &rhs) const
    {
      return data == rhs.data;
    }

    constexpr bool operator!=(nullptr_t) const
    {
      return data != nullptr;
    }

    constexpr bool operator!=(ref const &rhs) const
    {
      return data != rhs.data;
    }

    constexpr bool operator<(ref const &rhs) const
    {
      return data < rhs.data;
    }

    constexpr operator ref<value_type const>() const
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
