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
          if constexpr(std::is_void_v<R>)
          {
            fn(convert<Args, object_ptr>::call(args)...);
            return convert<R, object_ptr>::call();
          }
          else
          {
            return convert<R, object_ptr>::call(fn(convert<Args, object_ptr>::call(args)...));
          }
        }
      };
    }
  }
}
