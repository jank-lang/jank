#pragma once

#include <jank/runtime/convert.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/big_integer.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>

namespace jank::runtime
{
  template <typename T, typename... U>
  concept same_as_any = (std::same_as<T, U> || ...);

  template <>
  struct convert<void, object_ptr>
  {
    static object_ptr call()
    {
      return obj::nil::nil_const();
    }
  };

  template <typename T>
  requires behavior::object_like<T>
  struct convert<native_box<T>, object_ptr>
  {
    static object_ptr call(native_box<T> const o)
    {
      return o;
    }
  };

  template <typename T>
  requires behavior::object_like<T>
  struct convert<T *, object_ptr>
  {
    static object_ptr call(T * const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object *, object_ptr>
  {
    static object_ptr call(object * const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object const *, object_ptr>
  {
    static object_ptr call(object const * const o)
    {
      return const_cast<object *>(o);
    }
  };

  template <>
  struct convert<bool, object_ptr>
  {
    static object_ptr call(bool const o)
    {
      return make_box(o);
    }
  };

  /* Native integer primitives. */
  template <typename Input>
  requires(std::is_integral_v<Input>
           && !same_as_any<bool, char, char8_t, char16_t, char32_t, wchar_t>)
  struct convert<Input, object_ptr>
  {
    static object_ptr call(Input const o)
    {
      return make_box(o);
    }
  };

  /* Native floating point primitives. */
  template <typename Input>
  requires(std::is_floating_point_v<Input>)
  struct convert<Input, object_ptr>
  {
    static object_ptr call(Input const o)
    {
      return make_box(o);
    }
  };

  /* Native strings. */
  template <typename Input>
  requires(same_as_any<Input, native_persistent_string, native_persistent_string_view, std::string>)
  struct convert<Input, object_ptr>
  {
    static object_ptr call(Input const &o)
    {
      return make_box(o);
    }

    static object_ptr call(Input &&o)
    {
      return make_box(std::move(o));
    }
  };

  template <template <typename> typename V, typename Input>
  requires(
    convertible<Input, object_ptr>
    && (std::same_as<V<Input>, native_vector<Input>> || std::same_as<V<Input>, std::vector<Input>>))
  struct convert<V<Input>, object_ptr>
  {
    static object_ptr call(V<Input> const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<Input, object_ptr>::call(e));
      }
      return make_box<obj::persistent_vector>(trans);
    }
  };

  template <>
  struct convert<native_big_integer, object_ptr>
  {
    static object_ptr call(native_big_integer const &o)
    {
      return make_box<obj::big_integer>(o);
    }
  };
}
