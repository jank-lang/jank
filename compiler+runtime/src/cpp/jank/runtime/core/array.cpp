#include <jank/runtime/core/array.hpp>

#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  obj::bool_array_ref boolean_array(usize const size)
  {
    return make_box<obj::array<bool>>(array_element_type::boolean, size);
  }

  obj::bool_array_ref boolean_array(usize const size, bool const init_value)
  {
    return make_box<obj::array<bool>>(array_element_type::boolean, size, init_value);
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
    return make_box<obj::array<char>>(array_element_type::character, size);
  }

  obj::char_array_ref char_array(usize const size, char const init)
  {
    return make_box<obj::array<char>>(array_element_type::character, size, init);
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
    return make_box<obj::array<f32>>(array_element_type::f32, size);
  }

  obj::f32_array_ref float_array(usize const size, f32 const init)
  {
    return make_box<obj::array<f32>>(array_element_type::f32, size, init);
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
    return make_box<obj::array<f64>>(array_element_type::f64, size);
  }

  obj::f64_array_ref double_array(usize const size, f64 const init)
  {
    return make_box<obj::array<f64>>(array_element_type::f64, size, init);
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
    return make_box<obj::array<i16>>(array_element_type::i16, size);
  }

  obj::i16_array_ref short_array(usize const size, i16 const init)
  {
    return make_box<obj::array<i16>>(array_element_type::i16, size, init);
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
    return make_box<obj::array<i32>>(array_element_type::i32, size);
  }

  obj::i32_array_ref int_array(usize const size, i32 const init)
  {
    return make_box<obj::array<i32>>(array_element_type::i32, size, init);
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
    return make_box<obj::array<i64>>(array_element_type::i64, size);
  }

  obj::i64_array_ref long_array(usize const size, i64 const init)
  {
    return make_box<obj::array<i64>>(array_element_type::i64, size, init);
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
    return make_box<obj::array<object_ref>>(array_element_type::object, size);
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
    return make_box<obj::array<u8>>(array_element_type::u8, size);
  }

  obj::u8_array_ref byte_array(usize const size, u8 const init)
  {
    return make_box<obj::array<u8>>(array_element_type::u8, size, init);
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

  object_ref aget(object_ref const a, i64 const i)
  {
    if(a.get_type() != object_type::array)
    {
      throw std::runtime_error{ util::format("aget not supported on value of type: '{}'",
                                             object_type_str(a.get_type())) };
    }

    return a.nth(convert<i64>::into_object(i));
  }

  object_ref aset(object_ref a, i64 const i, object_ref const val)
  {
    return a.aset(i, val);
  }

  i64 alength(object_ref const a)
  {
    return static_cast<i64>(try_object<obj::array<i64>>(a)->size);
  }

  object_ref aclone(object_ref const a)
  {
    return a.aclone();
  }

  obj::bool_array_ref booleans(object_ref const o)
  {
    return try_array<bool>(array_element_type::boolean, o);
  }

  obj::char_array_ref chars(object_ref const o)
  {
    return try_array<char>(array_element_type::character, o);
  }

  obj::f32_array_ref floats(object_ref const o)
  {
    return try_array<f32>(array_element_type::f32, o);
  }

  obj::f64_array_ref doubles(object_ref const o)
  {
    return try_array<f64>(array_element_type::f64, o);
  }

  obj::i16_array_ref shorts(object_ref const o)
  {
    return try_array<i16>(array_element_type::i16, o);
  }

  obj::i32_array_ref ints(object_ref const o)
  {
    return try_array<i32>(array_element_type::i32, o);
  }

  obj::i64_array_ref longs(object_ref const o)
  {
    return try_array<i64>(array_element_type::i64, o);
  }

  obj::object_array_ref objects(object_ref const o)
  {
    return try_array<object_ref>(array_element_type::object, o);
  }

  obj::u8_array_ref bytes(object_ref const o)
  {
    return try_array<u8>(array_element_type::u8, o);
  }
}
