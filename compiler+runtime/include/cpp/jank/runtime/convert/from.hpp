#pragma once

#include <jank/runtime/convert.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  template <typename T>
  requires behavior::object_like<T>
  struct convert<object_ref, oref<T>>
  {
    static oref<T> call(object_ref const o)
    {
      return try_object<T>(o);
    }
  };

  template <typename T>
  requires behavior::object_like<T>
  struct convert<object_ref, T *>
  {
    static constexpr T *call(object_ref const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object_ref, object *>
  {
    static constexpr object *call(object_ref const o)
    {
      return o.data;
    }
  };

  template <>
  struct convert<object_ref, object const *>
  {
    static constexpr object const *call(object_ref const o)
    {
      return o.data;
    }
  };

  template <>
  struct convert<object_ref, bool>
  {
    static bool call(object_ref const o)
    {
      return try_object<obj::boolean>(o)->data;
    }
  };

  /* Native integer primitives. */
  template <typename Output>
  requires(std::is_integral_v<Output>
           && !jtl::is_any_same<bool, char, char8_t, char16_t, char32_t, wchar_t>)
  struct convert<object_ref, Output>
  {
    static Output call(object_ref const o)
    {
      return try_object<obj::integer>(o)->data;
    }
  };

  /* Native floating point primitives. */
  template <typename Output>
  requires(std::is_floating_point_v<Output>)
  struct convert<object_ref, Output>
  {
    static Output call(object_ref const o)
    {
      return try_object<obj::real>(o)->data;
    }
  };

  /* Native strings. */
  template <typename Output>
  requires(jtl::is_any_same<Output, jtl::immutable_string, std::string>)
  struct convert<object_ref, Output>
  {
    static Output call(object_ref const &o)
    {
      return try_object<obj::persistent_string>(o)->data;
    }
  };

  //template <template <typename> typename V, typename Output>
  //requires(
  //  convertible<object_ref, Output>
  //  && (std::same_as<V<Output>, native_vector<Output>> || std::same_as<V<Output>, std::vector<Output>>))
  //struct convert<object_ref, V<Output>>
  //{
  //  static V<Output> call(object_ref const &o)
  //  {
  //  }
  //};
}
