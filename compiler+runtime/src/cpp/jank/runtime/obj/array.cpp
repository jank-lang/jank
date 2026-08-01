#include <jank/runtime/obj/array.hpp>

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{
  array<bool> *array_box::booleans()
  {
    if(etype != element_type::boolean)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<bool> *>(this);
  }

  array<char> *array_box::chars()
  {
    if(etype != element_type::character)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<char> *>(this);
  }

  array<f32> *array_box::floats()
  {
    if(etype != element_type::f32)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<f32> *>(this);
  }

  array<f64> *array_box::doubles()
  {
    if(etype != element_type::f64)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<f64> *>(this);
  }

  array<i16> *array_box::shorts()
  {
    if(etype != element_type::i16)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<i16> *>(this);
  }

  array<i32> *array_box::ints()
  {
    if(etype != element_type::i32)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<i32> *>(this);
  }

  array<i64> *array_box::longs()
  {
    if(etype != element_type::i64)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<i64> *>(this);
  }

  array<object_ref> *array_box::objects()
  {
    if(etype != element_type::object)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<object_ref> *>(this);
  }

  array<u8> *array_box::bytes()
  {
    if(etype != element_type::u8)
    {
      throw std::runtime_error{ "boom" };
    }

    return reinterpret_cast<array<u8> *>(this);
  }

  object_ref array_box::operator[](std::size_t const i) const
  {
    if(i > size)
    {
      throw std::runtime_error{ "boom" };
    }

    switch(etype)
    {
      case element_type::boolean:
        return make_box(reinterpret_cast<array<bool> const *>(this)->data[i]);
      case element_type::character:
        return make_box(reinterpret_cast<array<char> const *>(this)->data[i]);
      case element_type::f32:
        return make_box(reinterpret_cast<array<f32> const *>(this)->data[i]);
      case element_type::f64:
        return make_box(reinterpret_cast<array<f64> const *>(this)->data[i]);
      case element_type::i16:
        return make_box(reinterpret_cast<array<i16> const *>(this)->data[i]);
      case element_type::i32:
        return make_box(reinterpret_cast<array<i32> const *>(this)->data[i]);
      case element_type::i64:
        return make_box(reinterpret_cast<array<i64> const *>(this)->data[i]);
      case element_type::object:
        return reinterpret_cast<array<object_ref> const *>(this)->data[i];
      case element_type::u8:
        return make_box(reinterpret_cast<array<u8> const *>(this)->data[i]);
      default:
        return make_box<opaque_box>(data /*.data + (i * element_size)*/, canonical_type);
    }
  }

  /* behavior::countable */
  usize array_box::count() const
  {
    return size;
  }

  object_ref array_box::get(object_ref const key) const
  {
    auto const n(static_cast<usize>(key.to_integer()));
    return operator[](n);
  }

  object_ref array_box::get(object_ref const key, object_ref const fallback) const
  {
    auto const n(static_cast<usize>(key.to_integer()));
    if(n > size)
    {
      return fallback;
    }
    return operator[](n);
  }

  bool array_box::contains(object_ref const key) const
  {
    auto const n(static_cast<usize>(key.to_integer()));
    return n < size;
  }

  element_type get_element_type(object_ref const o)
  {
    return try_object<obj::array_box>(o)->etype;
  }

  bool_array_ref boolean_array(u64 const size)
  {
    return make_box<array<bool>>(element_type::boolean, size);
  }

  bool_array_ref boolean_array(u64 const size, bool const init_value)
  {
    return make_box<array<bool>>(element_type::boolean, size, init_value);
  }

  char_array_ref char_array(u64 const size)
  {
    return make_box<array<char>>(element_type::character, size);
  }

  char_array_ref char_array(u64 const size, char const init)
  {
    return make_box<array<char>>(element_type::character, size, init);
  }

  f32_array_ref float_array(u64 const size)
  {
    return make_box<array<f32>>(element_type::f32, size);
  }

  f32_array_ref float_array(u64 const size, f32 const init)
  {
    return make_box<array<f32>>(element_type::f32, size, init);
  }

  f64_array_ref double_array(u64 const size)
  {
    return make_box<array<f64>>(element_type::f64, size);
  }

  f64_array_ref double_array(u64 const size, f64 const init)
  {
    return make_box<array<f64>>(element_type::f64, size, init);
  }

  i16_array_ref short_array(u64 const size)
  {
    return make_box<array<i16>>(element_type::i16, size);
  }

  i16_array_ref short_array(u64 const size, i16 const init)
  {
    return make_box<array<i16>>(element_type::i16, size, init);
  }

  i32_array_ref int_array(u64 const size)
  {
    return make_box<array<i32>>(element_type::i32, size);
  }

  i32_array_ref int_array(u64 const size, i32 const init)
  {
    return make_box<array<i32>>(element_type::i32, size, init);
  }

  i64_array_ref long_array(u64 const size)
  {
    return make_box<array<i64>>(element_type::i64, size);
  }

  i64_array_ref long_array(u64 const size, i64 const init)
  {
    return make_box<array<i64>>(element_type::i64, size, init);
  }

  object_array_ref object_array(u64 const size)
  {
    return make_box<array<object_ref>>(element_type::object, size);
  }

  u8_array_ref byte_array(u64 const size)
  {
    return make_box<array<u8>>(element_type::u8, size);
  }

  u8_array_ref byte_array(u64 const size, u8 const init)
  {
    return make_box<array<u8>>(element_type::u8, size, init);
  }
}
