#pragma once

#include <jank/runtime/convert.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/erasure.hpp>

namespace jank::runtime
{
  template <typename T>
  requires behavior::object_like<T>
  struct convert<object_ptr, native_box<T>>
  {
    static native_box<T> call(object_ptr const o)
    {
      return try_object<T>(o);
    }
  };

  template <typename T>
  requires behavior::object_like<T>
  struct convert<object_ptr, T *>
  {
    static T *call(object_ptr const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object_ptr, object *>
  {
    static object *call(object_ptr const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object_ptr, object const *>
  {
    static object const *call(object_ptr const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object_ptr, bool>
  {
    static bool call(object_ptr const o)
    {
      return try_object<obj::boolean>(o)->data;
    }
  };

  /* Native integer primitives. */
  template <typename Output>
  requires(std::is_integral_v<Output>
           && !same_as_any<bool, char, char8_t, char16_t, char32_t, wchar_t>)
  struct convert<object_ptr, Output>
  {
    static Output call(object_ptr const o)
    {
      return try_object<obj::integer>(o)->data;
    }
  };

  /* Native floating point primitives. */
  template <typename Output>
  requires(std::is_floating_point_v<Output>)
  struct convert<object_ptr, Output>
  {
    static Output call(object_ptr const o)
    {
      return try_object<obj::real>(o)->data;
    }
  };

  /* Native strings. */
  template <typename Output>
  requires(same_as_any<Output, native_persistent_string, std::string>)
  struct convert<object_ptr, Output>
  {
    static Output call(object_ptr const &o)
    {
      return try_object<obj::persistent_string>(o)->data;
    }
  };

  //template <template <typename> typename V, typename Output>
  //requires(
  //  convertible<object_ptr, Output>
  //  && (std::same_as<V<Output>, native_vector<Output>> || std::same_as<V<Output>, std::vector<Output>>))
  //struct convert<object_ptr, V<Output>>
  //{
  //  static V<Output> call(object_ptr const &o)
  //  {
  //  }
  //};
}
