#pragma once

#include <gc_cpp.h>

#include <jtl/primitive.hpp>
#include <jtl/trait/transform.hpp>
#include <jtl/assert.hpp>
#include <jtl/memory.hpp>

namespace jtl
{
  /* A `ref` is a non-owning, non-nullable box which must be always be initialized. The
   * memory referenced by a `ref` is expected to be owned elsewhere or GC allocated.
   *
   * This particular implementation of ref is only used for non-objects. There are two
   * noteworthy variations of ref in `object.hpp`:
   *
   * 1. oref<object>, which is a type-erased box which supports conversions from typed
   *    boxes
   * 2. oref<T> where T is object_like, which is a fully typed box
   *
   * Both of these above support a nil state, where the internal pointer
   * points at the nil constant. This can be done regardless of the ref type. */
  template <typename T>
  struct ref
  {
    using value_type = T;

    constexpr ref() = delete;
    constexpr ref(nullptr_t) = delete;
    constexpr ref(ref const &) noexcept = default;
    constexpr ref(ref &&) noexcept = default;

    constexpr ref(remove_const_t<value_type> * const data) noexcept
      : data{ data }
    {
      jank_debug_assert(this->data);
    }

    constexpr ref(value_type const * const data) noexcept
      : data{ const_cast<value_type *>(data) }
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

    constexpr ref &operator=(ref const &rhs) noexcept = default;
    constexpr ref &operator=(ref &&rhs) noexcept = default;

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
    return ret;
  }

  template <typename D, typename B>
  ref<D> static_ref_cast(ref<B> const r) noexcept
  {
    return static_cast<D *>(r.data);
  }
}
