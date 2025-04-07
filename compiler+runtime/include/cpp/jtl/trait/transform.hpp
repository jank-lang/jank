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
  struct decay
  {
    using type = T;
  };

  template <typename T>
  struct decay<T const>
  {
    using type = typename decay<T>::type;
  };

  template <typename T>
  struct decay<volatile T>
  {
    using type = typename decay<T>::type;
  };

  template <typename T>
  struct decay<T &>
  {
    using type = typename decay<T>::type;
  };

  template <typename T>
  struct decay<T &&>
  {
    using type = typename decay<T>::type;
  };

  template <typename T>
  using decay_t = typename decay<T>::type;
}
