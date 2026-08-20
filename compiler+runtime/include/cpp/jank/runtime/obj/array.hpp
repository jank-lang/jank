#pragma once

#include <utility>

#include <jtl/option.hpp>
#include <jtl/primitive.hpp>

#include <jank/runtime/convert/builtin.hpp>
#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/obj/opaque_box.hpp>
#include <jank/runtime/object.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{

  template <typename T>
  concept is_array_element_type
    = jtl::is_any_same<T, bool, char, u8, i8, u16, i16, u32, i32, u64, i64, f32, f64, object_ref>;

  template <typename T>
  constexpr array_element_type get_array_element_type()
  {
    if constexpr(jtl::is_same<T, bool>)
    {
      return array_element_type::boolean;
    }
    else if constexpr(jtl::is_same<T, char>)
    {
      return array_element_type::character;
    }
    else if constexpr(jtl::is_same<T, u8>)
    {
      return array_element_type::u8;
    }
    else if constexpr(jtl::is_same<T, i8>)
    {
      return array_element_type::i8;
    }
    else if constexpr(jtl::is_same<T, u16>)
    {
      return array_element_type::u16;
    }
    else if constexpr(jtl::is_same<T, i16>)
    {
      return array_element_type::i16;
    }
    else if constexpr(jtl::is_same<T, u32>)
    {
      return array_element_type::u32;
    }
    else if constexpr(jtl::is_same<T, i32>)
    {
      return array_element_type::i32;
    }
    else if constexpr(jtl::is_same<T, u64>)
    {
      return array_element_type::u64;
    }
    else if constexpr(jtl::is_same<T, i64>)
    {
      return array_element_type::i64;
    }
    else if constexpr(jtl::is_same<T, f32>)
    {
      return array_element_type::f32;
    }
    else if constexpr(jtl::is_same<T, f64>)
    {
      return array_element_type::f64;
    }
    else if constexpr(jtl::is_same<T, object_ref>)
    {
      return array_element_type::object;
    }
  }

  template <typename T>
  struct array : object
  {
    static constexpr object_type obj_type{ object_type::array };
    static constexpr object_behavior obj_behaviors{ object_behavior::get | object_behavior::seqable
                                                    | object_behavior::indexable
                                                    | object_behavior::array_like };
    static constexpr bool pointer_free{ false };

    array(usize const size)
      : object{ obj_type, obj_behaviors }
      , element_type{ get_array_element_type<T>() }
      , size{ size }
      , data{ new(UseGC) T[size]{} }
    {
    }

    array(usize const size, T init_value)
      : object{ obj_type, obj_behaviors }
      , element_type{ get_array_element_type<T>() }
      , size{ size }
      , data{ new(UseGC) T[size]{ init_value } }
    {
    }

    array_element_type get_element_type() const override
    {
      return element_type;
    }

    jtl::immutable_string to_code_string() const override
    {
      jtl::string_builder buff;
      util::format_to(buff, "#array [{} {} {}]", element_type_str(element_type), size, this);
      return buff.release();
    }

    object_ref aset(i64 const index, object_ref const val) override
    {
      auto const i(static_cast<usize>(index));
      operator[](i) = convert<T>::from_object(val);
      return val;
    }

    object_ref aclone() const override
    {
      auto const clone(make_box<array<T>>(size));
      std::copy(data.data, data.data + size, clone->data.data);
      return clone;
    }

    T &operator[](usize const i)
    {
      if(i >= size)
      {
        throw std::runtime_error{
          util::format("out of bounds index {}; array has a size of {}", i, size)
        };
      }

      return data[i];
    }

    T operator[](usize const i) const
    {
      if(i >= size)
      {
        throw std::runtime_error{
          util::format("out of bounds index {}; array has a size of {}", i, size)
        };
      }

      return data[i];
    }

    /* behavior::indexable */
    object_ref nth(object_ref const index) const override
    {
      auto const i(static_cast<usize>(index.to_integer()));
      if(i >= size)
      {
        throw std::runtime_error{
          util::format("out of bounds index {}; array has a size of {}", i, size)
        };
      }
      return convert<T>::into_object(data[i]);
    }

    object_ref nth(object_ref const index, object_ref const fallback) const override
    {
      auto const i(static_cast<usize>(index.to_integer()));
      if(i >= size)
      {
        return fallback;
      }
      return convert<T>::into_object(data[i]);
    }

    /* behavior::get */
    using object::get;

    object_ref get(object_ref const key) const override
    {
      auto const i(static_cast<usize>(key.to_integer()));
      if(i >= size)
      {
        return {};
      }
      return convert<T>::into_object(data[i]);
    }

    object_ref get(object_ref const key, object_ref const fallback) const override
    {
      auto const i(static_cast<usize>(key.to_integer()));
      if(i >= size)
      {
        return fallback;
      }
      return convert<T>::into_object(data[i]);
    }

    bool contains(object_ref const key) const override
    {
      auto const i(static_cast<usize>(key.to_integer()));
      return i < size;
    }

    /* behavior::seqable */
    object_ref seq() const override
    {
      if(size == 0)
      {
        return {};
      }

      auto const a(new(UseGC) object_ref[size]);

      for(usize i{}; i < size; ++i)
      {
        a[i] = convert<T>::into_object(data[i]);
      }

      return make_box<native_array_sequence>(a, size);
    }

    object_ref fresh_seq() const override
    {
      return seq();
    }

    /* behavior::countable */
    usize count() const
    {
      return size;
    }

    array_element_type element_type;
    usize size{};
    jtl::ptr<T> data{};
  };

  template <typename T>
  using array_ref = oref<struct array<T>>;

  using bool_array_ref = array_ref<bool>;
  using char_array_ref = array_ref<char>;
  using u8_array_ref = array_ref<u8>;
  using i8_array_ref = array_ref<i8>;
  using u16_array_ref = array_ref<u16>;
  using i16_array_ref = array_ref<i16>;
  using u32_array_ref = array_ref<u32>;
  using i32_array_ref = array_ref<i32>;
  using u64_array_ref = array_ref<u64>;
  using i64_array_ref = array_ref<i64>;
  using f32_array_ref = array_ref<f32>;
  using f64_array_ref = array_ref<f64>;
  using object_array_ref = array_ref<object_ref>;

  extern template struct array<bool>;
  extern template struct array<char>;
  extern template struct array<u8>;
  extern template struct array<i8>;
  extern template struct array<u16>;
  extern template struct array<i16>;
  extern template struct array<u32>;
  extern template struct array<i32>;
  extern template struct array<u64>;
  extern template struct array<i64>;
  extern template struct array<f32>;
  extern template struct array<f64>;
  extern template struct array<object_ref>;
}
