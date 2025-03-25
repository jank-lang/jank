#pragma once

#include <jtl/trait/predicate.hpp>

namespace jtl
{
  template <typename T>
  struct add_rvalue_reference
  {
    using type = T &&;
  };

  template <typename T>
  using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

  template <typename T>
  struct remove_reference
  {
    using type = T;
  };

  template <typename T>
  struct remove_reference<T &>
  {
    using type = T;
  };

  template <typename T>
  using remove_reference_t = typename remove_reference<T>::type;

  template <typename T>
  struct remove_const
  {
    using type = T;
  };

  template <typename T>
  struct remove_const<T const>
  {
    using type = T;
  };

  template <typename T>
  using remove_const_t = typename remove_const<T>::type;

  template <typename T>
  add_rvalue_reference_t<T> declval() noexcept
  {
    static_assert(false, "declval not allowed in an evaluated context");
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
