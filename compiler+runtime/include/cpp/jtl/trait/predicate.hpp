#pragma once

namespace jtl
{
  namespace detail
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
  }

  template <typename L, typename R>
  concept is_same = detail::is_same<L, R>::value;

  template <typename T>
  concept is_void = detail::is_same<T, void>::value;

  template <typename T>
  concept is_returnable = requires { static_cast<T (*)()>(nullptr); };

  template <typename From, typename To>
  concept is_implicitly_convertible = requires(void (&fn)(To), From &f) { fn(f); };

  template <typename From, typename To>
  concept is_convertible
    = (is_returnable<To> && is_implicitly_convertible<From, To>) || (is_void<From> && is_void<To>);

  template <typename T>
  concept is_lvalue_reference = is_same<T, T &>;

  template <typename T, typename... Args>
  concept is_constructible = __is_constructible(T, Args...);

  template <typename T>
  concept is_default_constructible = __is_constructible(T);

  template <typename T>
  concept is_move_constructible = __is_constructible(T, T);

  template <typename T>
  concept is_copy_constructible = __is_constructible(T, T const &);

  template <typename T>
  concept is_trivially_constructible = __is_trivially_constructible(T);

  template <typename T>
  concept is_trivially_copyable = __is_trivially_copyable(T);

  template <typename T>
  concept is_trivially_movable = __is_trivially_constructible(T, T);

  template <typename T>
  concept is_trivially_destructible = __is_trivially_destructible(T);
}
