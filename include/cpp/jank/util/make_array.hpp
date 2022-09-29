#pragma once

#include <array>

namespace jank::util
{
  namespace detail
  {
    template <typename>
    struct is_ref_wrapper : std::false_type {};
    template <typename T>
    struct is_ref_wrapper<std::reference_wrapper<T>> : std::true_type {};

    template<typename T>
    using not_ref_wrapper = std::negation<is_ref_wrapper<std::decay_t<T>>>;

    template <typename D, typename...>
    struct make_array_helper
    { using type = D; };
    template <typename... Args>
    struct make_array_helper<void, Args...> : std::common_type<Args...>
    {
      static_assert(std::conjunction_v<not_ref_wrapper<Args>...>,
                    "Args cannot contain reference_wrappers when D is void");
    };

    template <typename D, typename... Args>
    using make_array = std::array<typename make_array_helper<D, Args...>::type, sizeof...(Args)>;
  }

  template <typename D = void, typename... Args>
  constexpr detail::make_array<D, Args...> make_array(Args &&... ts)
  { return { std::forward<Args>(ts)... }; }
}
