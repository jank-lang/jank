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
        "Array cast failed, 'element_type' mismatch: expected '{}', received '{}'.",
        element_type_str(expected_element_type),
        element_type_str(actual_element_type)) };
    }

    return a;
  }

  extern template obj::bool_array_ref try_array(object_ref const o);
  extern template obj::char_array_ref try_array(object_ref const o);
  extern template obj::u8_array_ref try_array(object_ref const o);
  extern template obj::i8_array_ref try_array(object_ref const o);
  extern template obj::u16_array_ref try_array(object_ref const o);
  extern template obj::i16_array_ref try_array(object_ref const o);
  extern template obj::u32_array_ref try_array(object_ref const o);
  extern template obj::i32_array_ref try_array(object_ref const o);
  extern template obj::u64_array_ref try_array(object_ref const o);
  extern template obj::i64_array_ref try_array(object_ref const o);
  extern template obj::f32_array_ref try_array(object_ref const o);
  extern template obj::f64_array_ref try_array(object_ref const o);
  extern template obj::object_array_ref try_array(object_ref const o);

  template <typename T>
  T aget(obj::array_ref<T> const a, i64 const i)
  {
    if(i < 0 || static_cast<usize>(i) >= a->size)
    {
      throw std::runtime_error{
        util::format("out of bounds index {}; array has a size of {}", i, a->size)
      };
    }

    return a->data[i];
  }

  extern template bool aget(obj::bool_array_ref const a, i64 const i);
  extern template char aget(obj::char_array_ref const a, i64 const i);
  extern template u8 aget(obj::u8_array_ref const a, i64 const i);
  extern template i8 aget(obj::i8_array_ref const a, i64 const i);
  extern template u16 aget(obj::u16_array_ref const a, i64 const i);
  extern template i16 aget(obj::i16_array_ref const a, i64 const i);
  extern template u32 aget(obj::u32_array_ref const a, i64 const i);
  extern template i32 aget(obj::i32_array_ref const a, i64 const i);
  extern template u64 aget(obj::u64_array_ref const a, i64 const i);
  extern template i64 aget(obj::i64_array_ref const a, i64 const i);
  extern template f32 aget(obj::f32_array_ref const a, i64 const i);
  extern template f64 aget(obj::f64_array_ref const a, i64 const i);

  template <typename T, size_t N>
  T aget(T const (&a)[N], i64 const i)
  {
    if(i < 0 || std::cmp_greater_equal(i, N))
    {
      throw std::runtime_error{
        util::format("out of bounds index {}; array has a size of {}", i, N)
      };
    }

    return a[i];
  }

  object_ref aget(object_ref const a, i64 const i);

  template <typename T>
  T aset(obj::array_ref<T> const a, i64 const i, T const val)
  {
    if(i < 0 || static_cast<usize>(i) >= a->size)
    {
      throw std::runtime_error{
        util::format("out of bounds index {}; array has a size of {}", i, a->size)
      };
    }

    a->data[i] = val;

    return val;
  }

  extern template bool aset(obj::bool_array_ref const a, i64 const i, bool const val);
  extern template char aset(obj::char_array_ref const a, i64 const i, char const val);
  extern template u8 aset(obj::u8_array_ref const a, i64 const i, u8 const val);
  extern template i8 aset(obj::i8_array_ref const a, i64 const i, i8 const val);
  extern template u16 aset(obj::u16_array_ref const a, i64 const i, u16 const val);
  extern template i16 aset(obj::i16_array_ref const a, i64 const i, i16 const val);
  extern template u32 aset(obj::u32_array_ref const a, i64 const i, u32 const val);
  extern template i32 aset(obj::i32_array_ref const a, i64 const i, i32 const val);
  extern template u64 aset(obj::u64_array_ref const a, i64 const i, u64 const val);
  extern template i64 aset(obj::i64_array_ref const a, i64 const i, i64 const val);
  extern template f32 aset(obj::f32_array_ref const a, i64 const i, f32 const val);
  extern template f64 aset(obj::f64_array_ref const a, i64 const i, f64 const val);

  template <typename T>
  T aset(obj::array_ref<T> const a, i64 const i, object_ref const val)
  {
    return aset(a, i, convert<T>::from_object(val));
  }

  extern template bool aset(obj::bool_array_ref const a, i64 const i, object_ref const val);
  extern template char aset(obj::char_array_ref const a, i64 const i, object_ref const val);
  extern template u8 aset(obj::u8_array_ref const a, i64 const i, object_ref const val);
  extern template i8 aset(obj::i8_array_ref const a, i64 const i, object_ref const val);
  extern template u16 aset(obj::u16_array_ref const a, i64 const i, object_ref const val);
  extern template i16 aset(obj::i16_array_ref const a, i64 const i, object_ref const val);
  extern template u32 aset(obj::u32_array_ref const a, i64 const i, object_ref const val);
  extern template i32 aset(obj::i32_array_ref const a, i64 const i, object_ref const val);
  extern template u64 aset(obj::u64_array_ref const a, i64 const i, object_ref const val);
  extern template i64 aset(obj::i64_array_ref const a, i64 const i, object_ref const val);
  extern template f32 aset(obj::f32_array_ref const a, i64 const i, object_ref const val);
  extern template f64 aset(obj::f64_array_ref const a, i64 const i, object_ref const val);

  template <typename T, size_t N>
  T aset(T (&a)[N], i64 const i, T const val)
  {
    if(i < 0 || std::cmp_greater_equal(i, N))
    {
      throw std::runtime_error{
        util::format("out of bounds index {}; array has a size of {}", i, N)
      };
    }

    a[i] = val;
    return val;
  }

  template <typename T, size_t N>
  T aset(T (&a)[N], i64 const i, object_ref const val)
  {
    return aset(a, i, convert<T>::from_object(val));
  }

  object_ref aset(object_ref a, i64 const i, object_ref const val);

  template <typename T>
  usize alength(obj::array_ref<T> const a)
  {
    return static_cast<i64>(a->size);
  }

  extern template usize alength(obj::bool_array_ref const a);
  extern template usize alength(obj::char_array_ref const a);
  extern template usize alength(obj::u8_array_ref const a);
  extern template usize alength(obj::i8_array_ref const a);
  extern template usize alength(obj::u16_array_ref const a);
  extern template usize alength(obj::i16_array_ref const a);
  extern template usize alength(obj::u32_array_ref const a);
  extern template usize alength(obj::i32_array_ref const a);
  extern template usize alength(obj::u64_array_ref const a);
  extern template usize alength(obj::i64_array_ref const a);
  extern template usize alength(obj::f32_array_ref const a);
  extern template usize alength(obj::f64_array_ref const a);
  extern template usize alength(obj::object_array_ref const a);

  template <typename T, size_t N>
  usize alength(T const (&)[N])
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

  extern template obj::bool_array_ref aclone(obj::bool_array_ref const a);
  extern template obj::char_array_ref aclone(obj::char_array_ref const a);
  extern template obj::u8_array_ref aclone(obj::u8_array_ref const a);
  extern template obj::i8_array_ref aclone(obj::i8_array_ref const a);
  extern template obj::u16_array_ref aclone(obj::u16_array_ref const a);
  extern template obj::i16_array_ref aclone(obj::i16_array_ref const a);
  extern template obj::u32_array_ref aclone(obj::u32_array_ref const a);
  extern template obj::i32_array_ref aclone(obj::i32_array_ref const a);
  extern template obj::u64_array_ref aclone(obj::u64_array_ref const a);
  extern template obj::i64_array_ref aclone(obj::i64_array_ref const a);
  extern template obj::f32_array_ref aclone(obj::f32_array_ref const a);
  extern template obj::f64_array_ref aclone(obj::f64_array_ref const a);
  extern template obj::object_array_ref aclone(obj::object_array_ref const a);

  object_ref aclone(object_ref const a);

  template <typename T, size_t N>
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

  obj::bool_array_ref boolean_array(u64 const size);
  obj::bool_array_ref boolean_array(u64 const size, bool const init);
  obj::bool_array_ref boolean_array(object_ref const o);
  obj::char_array_ref char_array(u64 const size);
  obj::char_array_ref char_array(u64 const size, char const init);
  obj::char_array_ref char_array(jtl::immutable_string const &s);
  obj::char_array_ref char_array(object_ref const o);
  obj::f32_array_ref float_array(u64 const size);
  obj::f32_array_ref float_array(u64 const size, f32 const init);
  obj::f32_array_ref float_array(object_ref const o);
  obj::f64_array_ref double_array(u64 const size);
  obj::f64_array_ref double_array(u64 const size, f64 const init);
  obj::f64_array_ref double_array(object_ref const o);
  obj::i16_array_ref short_array(u64 const size);
  obj::i16_array_ref short_array(u64 const size, i16 const init);
  obj::i16_array_ref short_array(object_ref const o);
  obj::i32_array_ref int_array(u64 const size);
  obj::i32_array_ref int_array(u64 const size, i32 const init);
  obj::i32_array_ref int_array(object_ref const o);
  obj::i64_array_ref long_array(u64 const size);
  obj::i64_array_ref long_array(u64 const size, i64 const init);
  obj::i64_array_ref long_array(object_ref const o);
  obj::object_array_ref object_array(u64 const size);
  obj::object_array_ref object_array(object_ref const o);
  obj::u8_array_ref byte_array(u64 const size);
  obj::u8_array_ref byte_array(u64 const size, u8 const init);
  obj::u8_array_ref byte_array(object_ref const o);
}
