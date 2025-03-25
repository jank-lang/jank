#pragma once

#include <gc_cpp.h>

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

    constexpr ref(remove_const_t<value_type> &data) noexcept
      : data{ &data }
    {
      jank_debug_assert(this->data);
    }

    constexpr ref(value_type const &data) noexcept
      : data{ const_cast<value_type *>(&data) }
    {
      jank_debug_assert(this->data);
    }

    template <typename C>
    requires is_convertible<C *, T *>
    constexpr ref(ref<C> const data) noexcept
      : data{ data.data }
    {
      jank_debug_assert(this->data);
    }

    constexpr value_type *operator->() const noexcept
    {
      jank_debug_assert(data);
      return data;
    }

    constexpr value_type &operator*() const noexcept
    {
      jank_debug_assert(data);
      return *data;
    }

    constexpr bool operator==(ref const &rhs) const noexcept
    {
      return data == rhs.data;
    }

    constexpr bool operator!=(ref const &rhs) const noexcept
    {
      return data != rhs.data;
    }

    constexpr bool operator<(ref const &rhs) const noexcept
    {
      return data < rhs.data;
    }

    constexpr operator ref<value_type const>() const noexcept
    {
      return data;
    }

    //operator object *() const
    //{
    //  return &data->base;
    //}

    value_type *data{};
  };

  template <typename T>
  constexpr ref<T> make_ref(ref<T> const &o)
  {
    static_assert(sizeof(ref<T>) == sizeof(T *));
    return o;
  }

  /* TODO: Constexpr. */
  template <typename T, typename... Args>
  ref<T> make_ref(Args &&...args)
  {
    static_assert(sizeof(ref<T>) == sizeof(T *));
    T *ret{};
    if constexpr(requires { T::pointer_free; })
    {
      if constexpr(T::pointer_free)
      {
        ret = new(PointerFreeGC) T{ jtl::forward<Args>(args)... };
      }
      else
      {
        ret = new(GC) T{ jtl::forward<Args>(args)... };
      }
    }
    else
    {
      ret = new(GC) T{ jtl::forward<Args>(args)... };
    }

    jank_debug_assert(ret);
    return *ret;
  }

  template <typename D, typename B>
  ref<D> static_ref_cast(ref<B> const r) noexcept
  {
    return static_cast<D &>(*r);
  }
}
