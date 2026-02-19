#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  template <typename T>
  struct array : object
  {
    static constexpr object_type obj_type{ object_type::array };
    static constexpr object_behavior obj_behaviors{ object_behavior::get };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ false };

    array()
      : object{ obj_type, obj_behaviors }
    {
    }

    array(jtl::ref<T> const data, usize const size)
      : object{ obj_type, obj_behaviors }
      , data{ data }
      , size{ size }
    {
    }

    array(array &&) noexcept = default;
    array(array const &) = default;

    /* behavior::object_like */
    using object::to_string;

    void to_string(jtl::string_builder &buff) const override
    {
      buff(to_code_string());
    }

    /* behavior::seqable */
    //array_sequence_ref seq() const;
    //array_sequence_ref fresh_seq() const;

    /* behavior::countable */
    usize count() const
    {
      return size;
    }

    /* behavior::get */
    object_ref get(object_ref const key) const override
    {
      return get(key, {});
    }

    object_ref get(object_ref const key, object_ref const fallback) const override
    {
      if(key->type == object_type::integer)
      {
        auto const i(expect_object<integer>(key)->data);
        if(i < 0 || size <= static_cast<usize>(i))
        {
          return fallback;
        }
        return data.get()[i];
      }
      else
      {
        return fallback;
      }
    }

    bool contains(object_ref const key) const override
    {
      if(key->type == object_type::integer)
      {
        auto const i(expect_object<integer>(key)->data);
        return i >= 0 && static_cast<usize>(i) < size;
      }
      else
      {
        return false;
      }
    }

    /* behavior::indexable */
    object_ref nth(object_ref const index) const
    {
      if(index->type == object_type::integer)
      {
        auto const i(expect_object<integer>(index)->data);
        if(i < 0 || data.size() <= static_cast<usize>(i))
        {
          throw std::runtime_error{
            util::format("Array index {} is out of bounds, given size {}.", i, size)
          };
        }
        return data[i];
      }
      else
      {
        throw std::runtime_error{ util::format(
          "Argument to nth on a array must be an integer, not {}.",
          index->to_code_string()) };
      }
    }

    object_ref nth(object_ref const index, object_ref const fallback) const
    {
      return get(index, fallback);
    }

    /*** XXX: Everything here is NOT thread safe, but is not expected to be shared. ***/
    jtl::ref<T> data;
    usize size{};
  };
}
