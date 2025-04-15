#pragma once

#include <jank/runtime/convert/into.hpp>

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
          if constexpr(std::is_void_v<R>)
          {
            fn(convert<Args, object_ref>::call(args)...);
            return convert<R, object_ref>::call();
          }
          else
          {
            return convert<R, object_ref>::call(fn(convert<Args, object_ref>::call(args)...));
          }
        }
      };
    }
  }
}
