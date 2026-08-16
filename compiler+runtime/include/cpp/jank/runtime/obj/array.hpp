#pragma once

#include <jtl/option.hpp>
#include <jtl/primitive.hpp>

#include <jank/runtime/convert/builtin.hpp>
#include <jank/runtime/obj/opaque_box.hpp>
#include <jank/runtime/object.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  template <typename T>
  struct array_element_type_selector;

  template <>
  struct array_element_type_selector<bool>
  {
    static constexpr array_element_type element_type = array_element_type::boolean;
  };

  template <>
  struct array_element_type_selector<char>
  {
    static constexpr array_element_type element_type = array_element_type::character;
  };

  template <>
  struct array_element_type_selector<u8>
  {
    static constexpr array_element_type element_type = array_element_type::u8;
  };

  template <>
  struct array_element_type_selector<i8>
  {
    static constexpr array_element_type element_type = array_element_type::i8;
  };

  template <>
  struct array_element_type_selector<u16>
  {
    static constexpr array_element_type element_type = array_element_type::u16;
  };

  template <>
  struct array_element_type_selector<i16>
  {
    static constexpr array_element_type element_type = array_element_type::i16;
  };

  template <>
  struct array_element_type_selector<u32>
  {
    static constexpr array_element_type element_type = array_element_type::u32;
  };

  template <>
  struct array_element_type_selector<i32>
  {
    static constexpr array_element_type element_type = array_element_type::i32;
  };

  template <>
  struct array_element_type_selector<u64>
  {
    static constexpr array_element_type element_type = array_element_type::u64;
  };

  template <>
  struct array_element_type_selector<i64>
  {
    static constexpr array_element_type element_type = array_element_type::i64;
  };

  template <>
  struct array_element_type_selector<f32>
  {
    static constexpr array_element_type element_type = array_element_type::f32;
  };

  template <>
  struct array_element_type_selector<f64>
  {
    static constexpr array_element_type element_type = array_element_type::f64;
  };

  template <>
  struct array_element_type_selector<object_ref>
  {
    static constexpr array_element_type element_type = array_element_type::object;
  };

  template <typename T>
  struct array : object
  {
    static constexpr object_type obj_type{ object_type::array };
    static constexpr object_behavior obj_behaviors{ object_behavior::get | object_behavior::seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::indexable
                                                    | object_behavior::array_like };
    static constexpr bool pointer_free{ false };

    array(usize const size)
      : object{ obj_type, obj_behaviors }
      , element_type{ array_element_type_selector<T>::element_type }
      , size{ size }
      , data{ new(UseGC) T[size]{} }
    {
    }

    array(usize const size, T init_value)
      : object{ obj_type, obj_behaviors }
      , element_type{ array_element_type_selector<T>::element_type }
      , size{ size }
      , data{ new(UseGC) T[size]{} }
    {
      for(usize i{}; i < size; ++i)
      {
        data[i] = init_value;
      }
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

      for(usize i{}; i < size; ++i)
      {
        clone->data[i] = data[i];
      }

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
      return runtime::detail::untagged(this);
    }

    object_ref fresh_seq() const override
    {
      return runtime::detail::untagged(this);
    }

    /* behavior::sequence_like */
    object_ref first() const override
    {
      return convert<T>::into_object(data[0]);
    }

    object_ref next() const override
    {
      /* TODO: How should we implement this? */
      return {};
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
  using object_array_ref = array_ref<object_ref>;
  using f32_array_ref = array_ref<f32>;
  using f64_array_ref = array_ref<f64>;
}
