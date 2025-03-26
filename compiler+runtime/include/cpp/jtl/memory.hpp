#pragma once

#include <jtl/trait/transform.hpp>

namespace jtl
{
  [[nodiscard]]
  constexpr bool is_constexpr() noexcept
  {
    return __builtin_is_constant_evaluated();
  }

  template <class T>
  [[nodiscard, gnu::always_inline]]
  constexpr T *launder(T * const p) noexcept
  {
    /* XXX: It's UB to launder a function pointer or void pointer. */
    return __builtin_launder(p);
  }

  template <typename T>
  constexpr T &&declval() noexcept
  {
    static_assert(false, "declval not allowed in an evaluated context");
  }

  template <typename T>
  [[gnu::always_inline]]
  constexpr remove_reference_t<T> &&move(T &&value) noexcept
  {
    return static_cast<remove_reference_t<T> &&>(value);
  }

  /* TODO: Maybe put these in std? For cppcoreguidelines-missing-std-forward. */
  template <typename T>
  constexpr T &&forward(remove_reference_t<T> &t) noexcept
  {
    return static_cast<T &&>(t);
  }

  template <typename T>
  constexpr T &&forward(remove_reference_t<T> &&t) noexcept
  {
    static_assert(!is_lvalue_reference<T>,
                  "jtl::forward must not be used to convert an rvalue to an lvalue");
    return static_cast<T &&>(t);
  }
}
