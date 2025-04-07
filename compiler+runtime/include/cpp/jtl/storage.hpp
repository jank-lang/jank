#pragma once

#include <jtl/primitive.hpp>
#include <jtl/trait/predicate.hpp>
#include <jtl/memory.hpp>

namespace jtl
{
  /* This `storage` container allows us to store an instance of T in an in-place buffer.
   * Using a `storage` makes sense if you want to hold onto a value which may not be
   * default initialized. A good example of this is `option`.
   *
   * The `storage` container doesn't keep track of whether its value is set, so it doesn't
   * know when to destruct. This is meant to be explicitly controlled by whoever is using
   * the `storage` instead. See `option` for how this is done. */
  template <typename T>
  struct storage
  {
    storage() noexcept = default;

    template <typename... Args>
    requires is_constructible<T, Args...>
    constexpr explicit storage(Args &&...args) noexcept
    {
      new(value) T{ jtl::forward<Args>(args)... };
    }

    constexpr ~storage() noexcept = default;

    constexpr storage(storage const &src) noexcept
    requires is_trivially_copyable<T>
    = default;
    constexpr storage &operator=(storage const &) noexcept
    requires is_trivially_copyable<T>
    = default;

    constexpr storage(storage &&src) noexcept
    requires is_trivially_movable<T>
    = default;
    constexpr storage &operator=(storage &&) noexcept
    requires is_trivially_movable<T>
    = default;

    template <typename... Args>
    requires is_constructible<T, Args...>
    constexpr void construct(Args &&...args) noexcept
    {
      new(value) T{ jtl::forward<Args>(args)... };
    }

    constexpr void destruct() noexcept
    {
      if constexpr(!is_trivially_destructible<T>)
      {
        ptr()->~T();
      }
    }

    [[nodiscard]]
    constexpr T &operator*() noexcept
    {
      return *ptr();
    }

    [[nodiscard]]
    constexpr T const &operator*() const noexcept
    {
      return *ptr();
    }

    [[nodiscard]]
    constexpr T *operator->() noexcept
    {
      return ptr();
    }

    [[nodiscard]]
    constexpr T const *operator->() const noexcept
    {
      return ptr();
    }

    [[nodiscard]]
    constexpr u8 const *data() const noexcept
    {
      return value;
    }

    [[nodiscard]]
    constexpr u8 *data() noexcept
    {
      return value;
    }

    template <typename R = T>
    [[nodiscard]]
    constexpr R *ptr() noexcept
    {
      return jtl::launder(reinterpret_cast<R *>(value));
    }

    template <typename R = T>
    [[nodiscard]]
    constexpr R const *ptr() const noexcept
    {
      return jtl::launder(reinterpret_cast<T const *>(value));
    }

    template <typename R = T>
    [[nodiscard]]
    constexpr R& val() noexcept
    {
      return *ptr();
    }

    template <typename R = T>
    [[nodiscard]]
    constexpr R const& val() const noexcept
    {
      return *ptr();
    }

    constexpr storage& operator=(storage const &rhs) noexcept = default;
    constexpr storage& operator=(T &&rhs) noexcept
    {
      *ptr() = jtl::move(rhs);
      return *this;
    }

    /* NOLINTNEXTLINE(bugprone-sizeof-expression) */
    alignas(alignof(T)) u8 value[sizeof(T)]{};
  };
}
