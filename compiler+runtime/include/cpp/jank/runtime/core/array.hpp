#pragma once

#include <algorithm>
#include <utility>

#include <jtl/immutable_string.hpp>

#include <jank/runtime/obj/array.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  template <typename T>
  obj::array_ref<T> try_array(object_ref const o)
  {
    auto const a(try_object<obj::array<T>>(o));
    auto const expected_element_type(obj::get_array_element_type<T>());
    auto const actual_element_type(o.get_element_type());

    if(actual_element_type != expected_element_type)
    {
      throw std::runtime_error{ util::format(
        "An array of `{}` was needed here, but an array of `{}` was provided.",
        element_type_str(expected_element_type),
        element_type_str(actual_element_type)) };
    }

    return a;
  }

  template <typename T>
  T &aget(obj::array_ref<T> const a, i64 const i)
  {
    return (*a)[i];
  }

  template <typename T, usize N>
  T &aget(T (&a)[N], i64 const i)
  {
    if(i < 0 || std::cmp_greater_equal(i, N))
    {
      throw std::runtime_error{
        util::format("Index `{}` is out of bounds for an `array` of length `{}`.", i, N)
      };
    }

    return a[i];
  }

  object_ref aget(object_ref const a, i64 const i);

  template <typename T>
  T &aset(obj::array_ref<T> const a, i64 const i, T const val)
  {
    (*a)[i] = val;
    return (*a)[i];
  }

  template <typename T>
  T aset(obj::array_ref<T> const a, i64 const i, object_ref const val)
  {
    return aset(a, i, convert<T>::from_object(val));
  }

  template <typename T, usize N>
  T aset(T (&a)[N], i64 const i, T const val)
  {
    if(i < 0 || std::cmp_greater_equal(i, N))
    {
      throw std::runtime_error{
        util::format("Index `{}` is out of bounds for an `array` of length `{}`.", i, N)
      };
    }

    a[i] = val;
    return val;
  }

  template <typename T, usize N>
  T aset(T (&a)[N], i64 const i, object_ref const val)
  {
    return aset(a, i, convert<T>::from_object(val));
  }

  object_ref aset(object_ref a, i64 const i, object_ref const val);

  template <typename T>
  usize alength(obj::array_ref<T> const a)
  {
    return a->size;
  }

  template <typename T, usize N>
  usize alength(T (&)[N])
  {
    return N;
  }

  usize alength(object_ref const a);

  template <typename T>
  obj::array_ref<T> aclone(obj::array_ref<T> const a)
  {
    auto const size(a->size);
    auto const clone(make_box<obj::array<T>>(size));
    std::copy(a->data.data, a->data.data + size, clone->data.data);
    return clone;
  }

  object_ref aclone(object_ref const a);

  /* TODO: Move to `jank/runtime/convert/builtin.hpp`. Currently this causes a
   * circular dependency. */
  template <typename T, usize N>
  requires(!jtl::is_same<T, char> && obj::is_array_element_type<T>)
  struct convert<T[N]>
  {
    static obj::array_ref<T> into_object(T (&a)[N])
    {
      auto const clone(make_box<obj::array<T>>(N));
      std::copy(a, a + N, clone->data.data);
      return clone;
    }
  };

  obj::bool_array_ref booleans(object_ref const o);
  obj::char_array_ref chars(object_ref const o);
  obj::f32_array_ref floats(object_ref const o);
  obj::f64_array_ref doubles(object_ref const o);
  obj::i16_array_ref shorts(object_ref const o);
  obj::i32_array_ref ints(object_ref const o);
  obj::i64_array_ref longs(object_ref const o);
  obj::object_array_ref objects(object_ref const o);
  obj::u8_array_ref bytes(object_ref const o);

  obj::bool_array_ref boolean_array(i64 const size);
  obj::bool_array_ref boolean_array(i64 const size, bool const init);
  obj::bool_array_ref boolean_array(object_ref const o);
  obj::char_array_ref char_array(i64 const size);
  obj::char_array_ref char_array(i64 const size, char const init);
  obj::char_array_ref char_array(jtl::immutable_string const &s);
  obj::char_array_ref char_array(object_ref const o);
  obj::f32_array_ref float_array(i64 const size);
  obj::f32_array_ref float_array(i64 const size, f32 const init);
  obj::f32_array_ref float_array(object_ref const o);
  obj::f64_array_ref double_array(i64 const size);
  obj::f64_array_ref double_array(i64 const size, f64 const init);
  obj::f64_array_ref double_array(object_ref const o);
  obj::i16_array_ref short_array(i64 const size);
  obj::i16_array_ref short_array(i64 const size, i16 const init);
  obj::i16_array_ref short_array(object_ref const o);
  obj::i32_array_ref int_array(i64 const size);
  obj::i32_array_ref int_array(i64 const size, i32 const init);
  obj::i32_array_ref int_array(object_ref const o);
  obj::i64_array_ref long_array(i64 const size);
  obj::i64_array_ref long_array(i64 const size, i64 const init);
  obj::i64_array_ref long_array(object_ref const o);
  obj::object_array_ref object_array(i64 const size);
  obj::object_array_ref object_array(object_ref const o);
  obj::u8_array_ref byte_array(i64 const size);
  obj::u8_array_ref byte_array(i64 const size, u8 const init);
  obj::u8_array_ref byte_array(object_ref const o);
}
