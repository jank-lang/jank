#pragma once

#include <jtl/ref.hpp>

namespace jtl
{
  /* A `ptr` is a non-owning, nullable box. The memory referenced by a `ptr` is
   * expected to be owned elsewhere or GC allocated. */
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

    constexpr ptr(ref<value_type> const data)
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

    constexpr ref<value_type> as_ref() const
    {
      return data;
    }

    value_type *data{};
  };

  template <>
  struct ptr<void>
  {
    using value_type = void;

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
    requires is_convertible<C *, void *>
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

    constexpr operator value_type *() const
    {
      return data;
    }

    value_type *data{};
  };
}
