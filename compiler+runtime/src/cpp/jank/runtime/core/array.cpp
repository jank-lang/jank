#include <jank/runtime/core/array.hpp>

#include <jank/util/fmt.hpp>

#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime
{
  jtl::immutable_string get_element_type(object_ref const o)
  {
    return try_object<obj::array<int>>(o)->canonical_type;
  }

  obj::bool_array_ref boolean_array(u64 const size)
  {
    return make_box<obj::array<bool>>("bool", size);
  }

  obj::bool_array_ref boolean_array(u64 const size, bool const init_value)
  {
    return make_box<obj::array<bool>>("bool", size, init_value);
  }

  obj::bool_array_ref boolean_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::char_array_ref char_array(u64 const size)
  {
    return make_box<obj::array<char>>("char", size);
  }

  obj::char_array_ref char_array(u64 const size, char const init)
  {
    return make_box<obj::array<char>>("char", size, init);
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
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::f32_array_ref float_array(u64 const size)
  {
    return make_box<obj::array<f32>>("f32", size);
  }

  obj::f32_array_ref float_array(u64 const size, f32 const init)
  {
    return make_box<obj::array<f32>>("f32", size, init);
  }

  obj::f32_array_ref float_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::f64_array_ref double_array(u64 const size)
  {
    return make_box<obj::array<f64>>("f64", size);
  }

  obj::f64_array_ref double_array(u64 const size, f64 const init)
  {
    return make_box<obj::array<f64>>("f64", size, init);
  }

  obj::f64_array_ref double_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::i16_array_ref short_array(u64 const size)
  {
    return make_box<obj::array<i16>>("i16", size);
  }

  obj::i16_array_ref short_array(u64 const size, i16 const init)
  {
    return make_box<obj::array<i16>>("i16", size, init);
  }

  obj::i16_array_ref short_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::i32_array_ref int_array(u64 const size)
  {
    return make_box<obj::array<i32>>("i32", size);
  }

  obj::i32_array_ref int_array(u64 const size, i32 const init)
  {
    return make_box<obj::array<i32>>("i32", size, init);
  }

  obj::i32_array_ref int_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::i64_array_ref long_array(u64 const size)
  {
    return make_box<obj::array<i64>>("i64", size);
  }

  obj::i64_array_ref long_array(u64 const size, i64 const init)
  {
    return make_box<obj::array<i64>>("i64", size, init);
  }

  obj::i64_array_ref long_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::object_array_ref object_array(u64 const size)
  {
    return make_box<obj::array<object_ref>>("object", size);
  }

  obj::object_array_ref object_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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

  obj::u8_array_ref byte_array(u64 const size)
  {
    return make_box<obj::array<u8>>("u8", size);
  }

  obj::u8_array_ref byte_array(u64 const size, u8 const init)
  {
    return make_box<obj::array<u8>>("u8", size, init);
  }

  obj::u8_array_ref byte_array(object_ref const o)
  {
    if(o.get_type() == object_type::small_integer || o.get_type() == object_type::integer)
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
    auto const type(get_element_type(a));
    if(type == "bool")
    {
      return make_box(aget(expect_object<obj::array<bool>>(a), i));
    }
    else if(type == "char")
    {
      return make_box(aget(expect_object<obj::array<char>>(a), i));
    }
    else if(type == "f32")
    {
      return make_box(aget(expect_object<obj::array<f32>>(a), i));
    }
    else if(type == "f64")
    {
      return make_box(aget(expect_object<obj::array<f64>>(a), i));
    }
    else if(type == "i16")
    {
      return make_box(aget(expect_object<obj::array<i16>>(a), i));
    }
    else if(type == "i32")
    {
      return make_box(aget(expect_object<obj::array<i32>>(a), i));
    }
    else if(type == "i64")
    {
      return make_box(aget(expect_object<obj::array<i64>>(a), i));
    }
    else if(type == "object")
    {
      return make_box(aget(expect_object<obj::array<object_ref>>(a), i));
    }
    else if(type == "u8")
    {
      return make_box(aget(expect_object<obj::array<u8>>(a), i));
    }
    else
    {
      throw std::runtime_error{ util::format("aget not supported on value of type: '{}'",
                                             object_type_str(a.get_type())) };
    }
  }

  object_ref aset(object_ref const a, i64 const i, object_ref const val)
  {
    auto const type(get_element_type(a));
    if(type == "bool")
    {
      aset(expect_object<obj::array<bool>>(a), i, try_object<obj::boolean>(val)->data);
    }
    else if(type == "char")
    {
      aset(expect_object<obj::array<char>>(a), i, try_object<obj::character>(val)->data[0]);
    }
    else if(type == "f32")
    {
      aset(expect_object<obj::array<f32>>(a), i, static_cast<f32>(val.to_real()));
    }
    else if(type == "f64")
    {
      aset(expect_object<obj::array<f64>>(a), i, val.to_real());
    }
    else if(type == "i16")
    {
      aset(expect_object<obj::array<i16>>(a), i, static_cast<i16>(val.to_integer()));
    }
    else if(type == "i32")
    {
      aset(expect_object<obj::array<i32>>(a), i, static_cast<i32>(val.to_integer()));
    }
    else if(type == "i64")
    {
      aset(expect_object<obj::array<i64>>(a), i, val.to_integer());
    }
    else if(type == "object")
    {
      expect_object<obj::array<object_ref>>(a)->data[i] = val;
    }
    else if(type == "u8")
    {
      aset(expect_object<obj::array<u8>>(a), i, static_cast<u8>(val.to_integer()));
    }
    else
    {
      throw std::runtime_error{ util::format("aset not supported on value of type: '{}'",
                                             object_type_str(a.get_type())) };
    }

    return val;
  }

  i64 alength(object_ref const a)
  {
    return static_cast<i64>(try_object<obj::array<i64>>(a)->size);
  }

  object_ref aclone(object_ref const a)
  {
    auto const type(get_element_type(a));
    if(type == "bool")
    {
      return aclone(expect_object<obj::array<bool>>(a));
    }
    else if(type == "char")
    {
      return aclone(expect_object<obj::array<char>>(a));
    }
    else if(type == "f32")
    {
      return aclone(expect_object<obj::array<f32>>(a));
    }
    else if(type == "f64")
    {
      return aclone(expect_object<obj::array<f64>>(a));
    }
    else if(type == "i16")
    {
      return aclone(expect_object<obj::array<i16>>(a));
    }
    else if(type == "i32")
    {
      return aclone(expect_object<obj::array<i32>>(a));
    }
    else if(type == "i64")
    {
      return aclone(expect_object<obj::array<i64>>(a));
    }
    else if(type == "object")
    {
      return aclone(expect_object<obj::array<object_ref>>(a));
    }
    else if(type == "u8")
    {
      return aclone(expect_object<obj::array<u8>>(a));
    }
    else
    {
      throw std::runtime_error{ util::format("aclone not supported on value of type: {}",
                                             object_type_str(a.get_type())) };
    }
  }

  obj::bool_array_ref booleans(object_ref const o)
  {
    return try_array<bool>("bool", o);
  }

  obj::char_array_ref chars(object_ref const o)
  {
    return try_array<char>("char", o);
  }

  obj::f32_array_ref floats(object_ref const o)
  {
    return try_array<f32>("f32", o);
  }

  obj::f64_array_ref doubles(object_ref const o)
  {
    return try_array<f64>("f64", o);
  }

  obj::i16_array_ref shorts(object_ref const o)
  {
    return try_array<i16>("i16", o);
  }

  obj::i32_array_ref ints(object_ref const o)
  {
    return try_array<i32>("i32", o);
  }

  obj::i64_array_ref longs(object_ref const o)
  {
    return try_array<i64>("i64", o);
  }

  obj::object_array_ref objects(object_ref const o)
  {
    return try_array<object_ref>("object", o);
  }

  obj::u8_array_ref bytes(object_ref const o)
  {
    return try_array<u8>("u8", o);
  }
}
