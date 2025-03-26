#pragma once

#include <jtl/primitive.hpp>
#include <jtl/trait/predicate.hpp>
#include <jtl/memory.hpp>

namespace jtl
{
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

    [[nodiscard]]
    constexpr T *ptr() noexcept
    {
      return jtl::launder(reinterpret_cast<T *>(value));
    }

    [[nodiscard]]
    constexpr T const *ptr() const noexcept
    {
      return jtl::launder(reinterpret_cast<T const *>(value));
    }

    alignas(alignof(T)) u8 value[sizeof(T)]{};
  };
}
