#pragma once

#include <jank/runtime/convert.hpp>

namespace jank::runtime
{
  template <typename T>
  struct always_object_ptr
  {
    using type = object_ptr;
  };

  template <typename R, typename... Args>
  auto convert_function(R (* const fn)(Args...))
  {
    if constexpr(std::conjunction_v<std::is_same<object_ptr, R>, std::is_same<object_ptr, Args>...>)
    {
      return fn;
    }
    else
    {
      return std::function<object_ptr(typename always_object_ptr<Args>::type...)>{
        [fn](Args &&...args) -> object_ptr {
          return convert<R, object_ptr>::call(fn(convert<Args, object_ptr>::call(args)...));
        }
      };
    }
  }
}
