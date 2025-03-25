#pragma once

namespace jtl
{
  template <class T, class U>
  struct is_same
  {
    static constexpr bool value{ false };
  };

  template <class T>
  struct is_same<T, T>
  {
    static constexpr bool value{ true };
  };

  template <typename T>
  concept is_void = is_same<T, void>::value;

  template <typename T>
  concept is_returnable = requires { static_cast<T (*)()>(nullptr); };

  template <typename From, typename To>
  concept is_implicitly_convertible = requires { declval<void (&)(To)>()(declval<From>()); };

  template <typename From, typename To>
  concept is_convertible = requires {
    (is_returnable<To> && is_implicitly_convertible<From, To>) || (is_void<From> && is_void<To>);
  };

  template <typename T>
  concept is_lvalue_reference = is_same<T, T &>::value;
}
