#pragma once

#include <jank/runtime/convert.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>

namespace jank::runtime
{
  /* Any untyped object can convert to/from itself easily. */
  template <>
  struct convert<object_ref>
  {
    static object_ref into_object(object_ref const t)
    {
      return t;
    }

    static object_ref from_object(object_ref const t)
    {
      return t;
    }
  };

  template <typename T>
  requires(jtl::is_any_same<T, object *, object const *>)
  struct convert<T>
  {
    static object_ref into_object(T t)
    {
      return const_cast<object *>(t);
    }

    static object_ref into_object(object_ref const t)
    {
      return t.erase();
    }

    static object_ref from_object(T t)
    {
      return const_cast<object *>(t);
    }

    static object_ref from_object(object_ref const t)
    {
      return t.erase();
    }
  };

  /* Any typed object can convert to/from itself easily. */
  template <typename T>
  requires typed_object_ref<T>
  struct convert<T>
  {
    static T into_object(T const &t)
    {
      return t;
    }

    static T from_object(object_ref const t)
    {
      return try_object<typename T::value_type>(t);
    }

    static T from_object(T const t)
    {
      return t;
    }
  };

  /* Any type with conversion members can be convertible. */
  template <typename T>
  requires has_conversion_members<T>
  struct convert<T>
  {
    static auto into_object(T const &t)
    {
      return T::into_object(t);
    }

    static T from_object(auto const o)
    {
      return T::from_object(o);
    }
  };

  template <>
  struct convert<void>
  {
    static obj::nil_ref into_object()
    {
      return jank_nil();
    }

    static void from_object(object_ref const)
    {
    }
  };

  template <>
  struct convert<jtl::nullptr_t>
  {
    static obj::nil_ref into_object(jtl::nullptr_t)
    {
      return jank_nil();
    }

    static jtl::nullptr_t from_object(object_ref const)
    {
      return nullptr;
    }
  };

  template <>
  struct convert<bool>
  {
    static obj::boolean_ref into_object(bool const o)
    {
      return make_box(o);
    }

    static bool from_object(object_ref const o)
    {
      return try_object<obj::boolean>(o)->data;
    }

    static bool from_object(obj::boolean_ref const o)
    {
      return o->data;
    }
  };

  template <>
  struct convert<char>
  {
    static obj::character_ref into_object(char const o)
    {
      return make_box(o);
    }

    static char from_object(object_ref const o)
    {
      return try_object<obj::character>(o)->data[0];
    }

    static char from_object(obj::character_ref const o)
    {
      return o->data[0];
    }
  };

  template <typename T>
  requires std::is_enum_v<T>
  struct convert<T>
  {
    static obj::integer_ref into_object(T const o)
    {
      return make_box(static_cast<i64>(o));
    }

    static T from_object(object_ref const o)
    {
      return try_object<obj::integer>(o);
    }

    static T from_object(obj::integer_ref const o)
    {
      return o->data;
    }
  };

  /* Native integer primitives. */
  template <typename T>
  requires(std::is_integral_v<T>
           && !jtl::is_any_same<T, bool, char, char8_t, char16_t, char32_t, wchar_t>)
  struct convert<T>
  {
    static obj::integer_ref into_object(T const o)
    {
      return make_box(static_cast<i64>(o));
    }

    static T from_object(object_ref const o)
    {
      return static_cast<T>(try_object<obj::integer>(o)->data);
    }

    static T from_object(obj::integer_ref const o)
    {
      return static_cast<T>(o->data);
    }
  };

  /* Native big integer. */
  template <>
  struct convert<native_big_integer>
  {
    static obj::big_integer_ref into_object(native_big_integer const &o)
    {
      return make_box(o);
    }

    static native_big_integer from_object(object_ref const o)
    {
      return try_object<obj::big_integer>(o)->data;
    }

    static native_big_integer from_object(obj::big_integer_ref const o)
    {
      return o->data;
    }
  };

  /* Native floating point primitives. */
  template <typename T>
  requires(std::is_floating_point_v<T>)
  struct convert<T>
  {
    static obj::real_ref into_object(T const o)
    {
      return make_box(static_cast<f64>(o));
    }

    static T from_object(object_ref const o)
    {
      return static_cast<T>(try_object<obj::real>(o)->data);
    }

    static T from_object(obj::real_ref const o)
    {
      return static_cast<T>(o->data);
    }
  };

  /* C strings. */
  template <typename T>
  requires(jtl::is_any_same<std::decay_t<T>, char *, char const *>)
  struct convert<T>
  {
    static obj::persistent_string_ref into_object(char const * const o)
    {
      if(o == nullptr)
      {
        return jank_nil();
      }
      return make_box(o);
    }

    static char const *from_object(object_ref const o)
    {
      return try_object<obj::persistent_string>(o)->data.c_str();
    }

    static char const *from_object(obj::persistent_string_ref const o)
    {
      return o->data.c_str();
    }
  };

  /* Native strings. */
  template <typename T>
  requires(jtl::is_any_same<T, jtl::immutable_string, jtl::immutable_string_view, std::string>)
  struct convert<T>
  {
    static obj::persistent_string_ref into_object(T const &o)
    {
      return make_box(o);
    }

    static T from_object(object_ref const o)
    {
      return try_object<obj::persistent_string>(o)->data;
    }

    static T from_object(obj::persistent_string_ref const o)
    {
      return o->data;
    }
  };

  /* Support for both native_vector and std::vector of anything which itself is convertible. */
  template <template <typename> typename V, typename T>
  requires(convertible<T>
           && (jtl::is_same<V<T>, native_vector<T>> || jtl::is_same<V<T>, std::vector<T>>))
  struct convert<V<T>>
  {
    static obj::persistent_vector_ref into_object(V<T> const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans);
    }

    static V<T> from_object(object_ref const o)
    {
      return from_object(try_object<obj::persistent_string>(o));
    }

    static V<T> from_object(obj::persistent_vector_ref const o)
    {
      V<T> ret;
      for(auto const &e : o->data)
      {
        ret.push_back(e);
      }
      return ret;
    }
  };
}
