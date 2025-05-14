#pragma once

#include <jank/runtime/convert/builtin.hpp>

namespace jank::runtime
{
  template <typename T>
  struct always_object_ref
  {
    using type = object_ref;
  };

  template <typename R, typename... Args>
  auto convert_function(R (* const fn)(Args...))
  {
    if constexpr(std::conjunction_v<std::is_same<object_ref, R>, std::is_same<object_ref, Args>...>)
    {
      return fn;
    }
    else
    {
      return std::function<object_ref(typename always_object_ref<Args>::type...)>{
        [fn](Args &&...args) -> object_ref {
          if constexpr(jtl::is_void<R>)
          {
            fn(convert<Args>::into_object(jtl::forward<Args>(args))...);
            return convert<R>::into_object();
          }
          else
          {
            return convert<R>::into_object(
              fn(convert<Args>::into_object(jtl::forward<Args>(args))...));
          }
        }
      };
    }
  }
}
