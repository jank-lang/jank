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
  struct array : object
  {
    static constexpr object_type obj_type{ object_type::array };
    static constexpr object_behavior obj_behaviors{ object_behavior::get | object_behavior::seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::indexable };
    static constexpr bool pointer_free{ false };

    array(jtl::immutable_string const &canonical_type, usize const size)
      : object{ obj_type, obj_behaviors }
      , canonical_type{ canonical_type }
      , size{ size }
      , data{ new(UseGC) T[size]{} }
    {
    }

    array(jtl::immutable_string const &canonical_type, usize const size, T init_value)
      : object{ obj_type, obj_behaviors }
      , canonical_type{ canonical_type }
      , size{ size }
      , data{ new(UseGC) T[size]{} }
    {
      for(usize i{}; i < size; ++i)
      {
        data[i] = init_value;
      }
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

    jtl::immutable_string canonical_type{};
    usize size{};
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
}
