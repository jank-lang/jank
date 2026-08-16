#include <jank/runtime/core/array.hpp>

#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  template obj::bool_array_ref try_array(object_ref const o);
  template obj::char_array_ref try_array(object_ref const o);
  template obj::u8_array_ref try_array(object_ref const o);
  template obj::i8_array_ref try_array(object_ref const o);
  template obj::u16_array_ref try_array(object_ref const o);
  template obj::i16_array_ref try_array(object_ref const o);
  template obj::u32_array_ref try_array(object_ref const o);
  template obj::i32_array_ref try_array(object_ref const o);
  template obj::u64_array_ref try_array(object_ref const o);
  template obj::i64_array_ref try_array(object_ref const o);
  template obj::f32_array_ref try_array(object_ref const o);
  template obj::f64_array_ref try_array(object_ref const o);
  template obj::object_array_ref try_array(object_ref const o);

  obj::bool_array_ref boolean_array(usize const size)
  {
    return make_box<obj::array<bool>>(size);
  }

  obj::bool_array_ref boolean_array(usize const size, bool const init_value)
  {
    return make_box<obj::array<bool>>(size, init_value);
  }

  obj::bool_array_ref boolean_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return boolean_array(o.to_integer());
    }

    auto a(boolean_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = try_object<obj::boolean>(e)->data;
      ++i;
    }

    return a;
  }

  obj::char_array_ref char_array(usize const size)
  {
    return make_box<obj::array<char>>(size);
  }

  obj::char_array_ref char_array(usize const size, char const init)
  {
    return make_box<obj::array<char>>(size, init);
  }

  obj::char_array_ref char_array(jtl::immutable_string const &s)
  {
    auto const size(s.size());
    auto a(char_array(size));

    for(usize i{}; i < size; ++i)
    {
      a->data[i] = s[i];
    }

    return a;
  }

  obj::char_array_ref char_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return char_array(o.to_integer());
    }

    if(o.get_type() == object_type::persistent_string)
    {
      return char_array(expect_object<obj::persistent_string>(o)->data);
    }

    auto a(char_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = try_object<obj::character>(e)->data[0];
      ++i;
    }

    return a;
  }

  obj::f32_array_ref float_array(usize const size)
  {
    return make_box<obj::array<f32>>(size);
  }

  obj::f32_array_ref float_array(usize const size, f32 const init)
  {
    return make_box<obj::array<f32>>(size, init);
  }

  obj::f32_array_ref float_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return float_array(o.to_integer());
    }

    auto a(float_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = static_cast<f32>(e.to_real());
      ++i;
    }

    return a;
  }

  obj::f64_array_ref double_array(usize const size)
  {
    return make_box<obj::array<f64>>(size);
  }

  obj::f64_array_ref double_array(usize const size, f64 const init)
  {
    return make_box<obj::array<f64>>(size, init);
  }

  obj::f64_array_ref double_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return double_array(o.to_integer());
    }

    auto a(double_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = e.to_real();
      ++i;
    }

    return a;
  }

  obj::i16_array_ref short_array(usize const size)
  {
    return make_box<obj::array<i16>>(size);
  }

  obj::i16_array_ref short_array(usize const size, i16 const init)
  {
    return make_box<obj::array<i16>>(size, init);
  }

  obj::i16_array_ref short_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return short_array(o.to_integer());
    }

    auto a(short_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = static_cast<i16>(e.to_integer());
      ++i;
    }

    return a;
  }

  obj::i32_array_ref int_array(usize const size)
  {
    return make_box<obj::array<i32>>(size);
  }

  obj::i32_array_ref int_array(usize const size, i32 const init)
  {
    return make_box<obj::array<i32>>(size, init);
  }

  obj::i32_array_ref int_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return int_array(o.to_integer());
    }

    auto a(int_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = static_cast<i32>(e.to_integer());
      ++i;
    }

    return a;
  }

  obj::i64_array_ref long_array(usize const size)
  {
    return make_box<obj::array<i64>>(size);
  }

  obj::i64_array_ref long_array(usize const size, i64 const init)
  {
    return make_box<obj::array<i64>>(size, init);
  }

  obj::i64_array_ref long_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return long_array(o.to_integer());
    }

    auto a(long_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = e.to_integer();
      ++i;
    }

    return a;
  }

  obj::object_array_ref object_array(usize const size)
  {
    return make_box<obj::array<object_ref>>(size);
  }

  obj::object_array_ref object_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return object_array(o.to_integer());
    }

    auto a(object_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = e;
      ++i;
    }

    return a;
  }

  obj::u8_array_ref byte_array(usize const size)
  {
    return make_box<obj::array<u8>>(size);
  }

  obj::u8_array_ref byte_array(usize const size, u8 const init)
  {
    return make_box<obj::array<u8>>(size, init);
  }

  obj::u8_array_ref byte_array(object_ref const o)
  {
    if(is_integer(o))
    {
      return byte_array(o.to_integer());
    }

    auto a(byte_array(sequence_length(o)));

    usize i{};
    for(auto const e : make_sequence_range(o))
    {
      a->data[i] = e.to_integer();
      ++i;
    }

    return a;
  }

  template bool aget(obj::bool_array_ref const a, i64 const i);
  template char aget(obj::char_array_ref const a, i64 const i);
  template u8 aget(obj::u8_array_ref const a, i64 const i);
  template i8 aget(obj::i8_array_ref const a, i64 const i);
  template u16 aget(obj::u16_array_ref const a, i64 const i);
  template i16 aget(obj::i16_array_ref const a, i64 const i);
  template u32 aget(obj::u32_array_ref const a, i64 const i);
  template i32 aget(obj::i32_array_ref const a, i64 const i);
  template u64 aget(obj::u64_array_ref const a, i64 const i);
  template i64 aget(obj::i64_array_ref const a, i64 const i);
  template f32 aget(obj::f32_array_ref const a, i64 const i);
  template f64 aget(obj::f64_array_ref const a, i64 const i);

  object_ref aget(object_ref const a, i64 const i)
  {
    if(a.get_type() != object_type::array)
    {
      throw std::runtime_error{ util::format("aget not supported on value of type: '{}'",
                                             object_type_str(a.get_type())) };
    }

    return a.nth(convert<i64>::into_object(i));
  }

  template bool aset(obj::bool_array_ref const a, i64 const i, bool const val);
  template char aset(obj::char_array_ref const a, i64 const i, char const val);
  template u8 aset(obj::u8_array_ref const a, i64 const i, u8 const val);
  template i8 aset(obj::i8_array_ref const a, i64 const i, i8 const val);
  template u16 aset(obj::u16_array_ref const a, i64 const i, u16 const val);
  template i16 aset(obj::i16_array_ref const a, i64 const i, i16 const val);
  template u32 aset(obj::u32_array_ref const a, i64 const i, u32 const val);
  template i32 aset(obj::i32_array_ref const a, i64 const i, i32 const val);
  template u64 aset(obj::u64_array_ref const a, i64 const i, u64 const val);
  template i64 aset(obj::i64_array_ref const a, i64 const i, i64 const val);
  template f32 aset(obj::f32_array_ref const a, i64 const i, f32 const val);
  template f64 aset(obj::f64_array_ref const a, i64 const i, f64 const val);

  template bool aset(obj::bool_array_ref const a, i64 const i, object_ref const val);
  template char aset(obj::char_array_ref const a, i64 const i, object_ref const val);
  template u8 aset(obj::u8_array_ref const a, i64 const i, object_ref const val);
  template i8 aset(obj::i8_array_ref const a, i64 const i, object_ref const val);
  template u16 aset(obj::u16_array_ref const a, i64 const i, object_ref const val);
  template i16 aset(obj::i16_array_ref const a, i64 const i, object_ref const val);
  template u32 aset(obj::u32_array_ref const a, i64 const i, object_ref const val);
  template i32 aset(obj::i32_array_ref const a, i64 const i, object_ref const val);
  template u64 aset(obj::u64_array_ref const a, i64 const i, object_ref const val);
  template i64 aset(obj::i64_array_ref const a, i64 const i, object_ref const val);
  template f32 aset(obj::f32_array_ref const a, i64 const i, object_ref const val);
  template f64 aset(obj::f64_array_ref const a, i64 const i, object_ref const val);

  object_ref aset(object_ref a, i64 const i, object_ref const val)
  {
    return a.aset(i, val);
  }

  template i64 alength(obj::bool_array_ref const a);
  template i64 alength(obj::char_array_ref const a);
  template i64 alength(obj::u8_array_ref const a);
  template i64 alength(obj::i8_array_ref const a);
  template i64 alength(obj::u16_array_ref const a);
  template i64 alength(obj::i16_array_ref const a);
  template i64 alength(obj::u32_array_ref const a);
  template i64 alength(obj::i32_array_ref const a);
  template i64 alength(obj::u64_array_ref const a);
  template i64 alength(obj::i64_array_ref const a);
  template i64 alength(obj::f32_array_ref const a);
  template i64 alength(obj::f64_array_ref const a);
  template i64 alength(obj::object_array_ref const a);

  i64 alength(object_ref const a)
  {
    return static_cast<i64>(try_object<obj::array<i64>>(a)->size);
  }

  template obj::bool_array_ref aclone(obj::bool_array_ref const a);
  template obj::char_array_ref aclone(obj::char_array_ref const a);
  template obj::u8_array_ref aclone(obj::u8_array_ref const a);
  template obj::i8_array_ref aclone(obj::i8_array_ref const a);
  template obj::u16_array_ref aclone(obj::u16_array_ref const a);
  template obj::i16_array_ref aclone(obj::i16_array_ref const a);
  template obj::u32_array_ref aclone(obj::u32_array_ref const a);
  template obj::i32_array_ref aclone(obj::i32_array_ref const a);
  template obj::u64_array_ref aclone(obj::u64_array_ref const a);
  template obj::i64_array_ref aclone(obj::i64_array_ref const a);
  template obj::f32_array_ref aclone(obj::f32_array_ref const a);
  template obj::f64_array_ref aclone(obj::f64_array_ref const a);
  template obj::object_array_ref aclone(obj::object_array_ref const a);

  object_ref aclone(object_ref const a)
  {
    return a.aclone();
  }

  obj::bool_array_ref booleans(object_ref const o)
  {
    return try_array<bool>(o);
  }

  obj::char_array_ref chars(object_ref const o)
  {
    return try_array<char>(o);
  }

  obj::f32_array_ref floats(object_ref const o)
  {
    return try_array<f32>(o);
  }

  obj::f64_array_ref doubles(object_ref const o)
  {
    return try_array<f64>(o);
  }

  obj::i16_array_ref shorts(object_ref const o)
  {
    return try_array<i16>(o);
  }

  obj::i32_array_ref ints(object_ref const o)
  {
    return try_array<i32>(o);
  }

  obj::i64_array_ref longs(object_ref const o)
  {
    return try_array<i64>(o);
  }

  obj::object_array_ref objects(object_ref const o)
  {
    return try_array<object_ref>(o);
  }

  obj::u8_array_ref bytes(object_ref const o)
  {
    return try_array<u8>(o);
  }
}
