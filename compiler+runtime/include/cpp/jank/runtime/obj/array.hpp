#pragma once

#include <jtl/option.hpp>
#include <jtl/primitive.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/opaque_box.hpp>

namespace jank::runtime::obj
{
  enum class element_type : u8
  {
    boolean,
    character,
    f32,
    f64,
    i16,
    i32,
    i64,
    object,
    u8,
  };

  using array_box_ref = oref<struct array_box>;

  template <typename T>
  struct array : object
  {
    static constexpr object_type obj_type{ object_type::array_box };
    static constexpr object_behavior obj_behaviors{ object_behavior::get };
    static constexpr bool pointer_free{ false };

    array(element_type const etype, usize const size)
      : object{ obj_type, obj_behaviors }
      , etype{ etype }
      , size{ size }
      , data{ new(UseGC) T[size] }
    {
    }

    array(element_type const etype, usize const size, T init_value)
      : object{ obj_type, obj_behaviors }
      , etype{ etype }
      , size{ size }
      , data{ new(UseGC) T[size] }
    {
      for(usize i{}; i < size; ++i)
      {
        data[i] = init_value;
      }
    }

    T &operator[](std::size_t const i)
    {
      if(i > size)
      {
        throw std::runtime_error{ "boom" };
      }
      return data[i];
    }

    T operator[](std::size_t const i) const
    {
      if(i > size)
      {
        throw std::runtime_error{ "boom" };
      }
      return data[i];
    }

    T at(std::size_t i) const
    {
      if(i > size)
      {
        throw std::runtime_error{ "boom" };
      }
      return data[i];
    }

    /* behavior::get */
    using object::get;

    object_ref get(object_ref const key) const override
    {
      auto const n(static_cast<usize>(key.to_integer()));
      return make_box(operator[](n));
    }

    object_ref get(object_ref const key, object_ref const fallback)
    {
      auto const n(static_cast<usize>(key.to_integer()));
      if(n > size)
      {
        return fallback;
      }
      return make_box(operator[](n));
    }

    bool contains(object_ref const key) const override
    {
      auto const n(static_cast<usize>(key.to_integer()));
      return n < size;
    }

    array_box_ref into_object()
    {
      return reinterpret_cast<array_box *>(this);
    }

    element_type etype;
    usize size;
    jtl::immutable_string canonical_type{};
    jtl::ptr<T> data{};
  };

  template <typename T>
  using array_ref = oref<struct array<T>>;

  using bool_array_ref = array_ref<bool>;
  using char_array_ref = array_ref<char>;
  using f32_array_ref = array_ref<f32>;
  using f64_array_ref = array_ref<f64>;
  using i16_array_ref = array_ref<i16>;
  using i32_array_ref = array_ref<i32>;
  using i64_array_ref = array_ref<i64>;
  using object_array_ref = array_ref<object_ref>;
  using u8_array_ref = array_ref<u8>;

  struct array_box : object
  {
    static constexpr object_type obj_type{ object_type::array_box };
    static constexpr object_behavior obj_behaviors{ object_behavior::get };
    static constexpr bool pointer_free{ false };

    object_ref operator[](std::size_t const i) const;

    array<bool> *booleans();
    array<char> *chars();
    array<f32> *floats();
    array<f64> *doubles();
    array<i16> *shorts();
    array<i32> *ints();
    array<i64> *longs();
    array<object_ref> *objects();
    array<u8> *bytes();

    /* behavior::countable */
    usize count() const;

    /* behavior::get */
    using object::get;
    object_ref get(object_ref const key) const override;
    object_ref get(object_ref const key, object_ref const fallback) const override;
    bool contains(object_ref const key) const override;

    element_type etype;
    usize size;
    jtl::immutable_string canonical_type{};
    jtl::ptr<void> data{};
  };

  element_type get_element_type(object_ref const o);

  bool_array_ref boolean_array(u64 const size);
  bool_array_ref boolean_array(u64 const size, bool const init);
  char_array_ref char_array(u64 const size);
  char_array_ref char_array(u64 const size, char const init);
  f32_array_ref float_array(u64 const size);
  f32_array_ref float_array(u64 const size, f32 const init);
  f64_array_ref double_array(u64 const size);
  f64_array_ref double_array(u64 const size, f64 const init);
  i16_array_ref short_array(u64 const size);
  i16_array_ref short_array(u64 const size, i16 const init);
  i32_array_ref int_array(u64 const size);
  i32_array_ref int_array(u64 const size, i32 const init);
  i64_array_ref long_array(u64 const size);
  i64_array_ref long_array(u64 const size, i64 const init);
  object_array_ref object_array(u64 const size);
  u8_array_ref byte_array(u64 const size);
  u8_array_ref byte_array(u64 const size, u8 const init);
}
