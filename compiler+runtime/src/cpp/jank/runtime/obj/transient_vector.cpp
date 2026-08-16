#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  transient_vector::transient_vector()
    : object{ obj_type, obj_behaviors }
  {
  }

  transient_vector::transient_vector(runtime::detail::native_persistent_vector &&d)
    : object{ obj_type, obj_behaviors }
    , data{ std::move(d).transient() }
  {
  }

  transient_vector::transient_vector(runtime::detail::native_persistent_vector const &d)
    : object{ obj_type, obj_behaviors }
    , data{ d.transient() }
  {
  }

  transient_vector::transient_vector(runtime::detail::native_transient_vector &&d)
    : object{ obj_type, obj_behaviors }
    , data{ std::move(d) }
  {
  }

  transient_vector_ref transient_vector::empty()
  {
    return make_box<transient_vector>();
  }

  usize transient_vector::count() const
  {
    assert_active();
    return data.size();
  }

  transient_vector_ref transient_vector::conj_in_place(object_ref const head)
  {
    assert_active();
    data.push_back(head);
    return runtime::detail::untagged(this);
  }

  object_ref transient_vector::nth(object_ref const index) const
  {
    if(is_integral(index))
    {
      auto const i(to_i64(index));
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        throw std::runtime_error{ util::format(
          "The index `{}` is out of bounds for this `transient_vector` of length `{}`.",
          i,
          data.size()) };
      }
      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `nth` operation on a `transient_vector` requires an integer index, not a `{}`.",
        object_type_str(index.get_type())) };
    }
  }

  object_ref transient_vector::nth(object_ref const index, object_ref const fallback) const
  {
    return get(index, fallback);
  }

  transient_vector_ref transient_vector::assoc_in_place(object_ref const key, object_ref const val)
  {
    assert_active();
    if(!is_integral(key))
    {
      throw std::runtime_error{ util::format(
        "The `assoc` operation on a `transient_vector` requires an integer key, not a `{}`.",
        object_type_str(key.get_type())) };
    }

    auto const i(to_i64(key));
    auto const size(static_cast<i64>(data.size()));

    if(i > size || 0 > i)
    {
      throw std::runtime_error{ util::format(
        "The index `{}` is out of bounds for this `transient_vector` of length `{}`.",
        i,
        data.size()) };
    }
    else if(i == size)
    {
      data.push_back(val);
    }
    else
    {
      data.set(i, val);
    }

    return runtime::detail::untagged(this);
  }

  transient_vector_ref transient_vector::dissoc_in_place(object_ref const /*key*/)
  {
    throw std::runtime_error{ "The `dissoc` operation on a `transient_vector` is not supported." };
  }

  transient_vector::persistent_type_ref transient_vector::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_vector>(data.persistent());
  }

  transient_vector::persistent_type_ref
  transient_vector::to_persistent(jtl::immutable_string const &meta)
  {
    assert_active();
    active = false;
    return make_box<persistent_vector>(meta, data.persistent());
  }

  object_ref transient_vector::call(object_ref const idx) const
  {
    assert_active();
    if(is_integral(idx))
    {
      auto const i(to_i64(idx));
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        throw std::runtime_error{ util::format(
          "The index `{}` is out of bounds for this `transient_vector` of length `{}`.",
          i,
          data.size()) };
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `call` operation on a `transient_vector` requires an integer index, not a `{}`.",
        object_type_str(idx.get_type())) };
    }
  }

  object_ref transient_vector::get(object_ref const idx) const
  {
    assert_active();
    if(is_integral(idx))
    {
      auto const i(to_i64(idx));
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return {};
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `get` operation on a `transient_vector` requires an integer index, not a `{}`.",
        object_type_str(idx.get_type())) };
    }
  }

  object_ref transient_vector::get(object_ref const idx, object_ref const fallback) const
  {
    assert_active();
    if(is_integral(idx))
    {
      auto const i(to_i64(idx));
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `get` operation on a `transient_vector` requires an integer index, not a `{}`.",
        object_type_str(idx.get_type())) };
    }
  }

  object_ref transient_vector::find(object_ref const idx) const
  {
    if(is_integral(idx))
    {
      auto const i(to_i64(idx));
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return {};
      }
      /* TODO: Map entry type? */
      return make_box<persistent_vector>(std::in_place, idx, data[i]);
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `find` operation on a `transient_vector` requires an integer index, not a `{}`.",
        object_type_str(idx.get_type())) };
    }
  }

  bool transient_vector::contains(object_ref const elem) const
  {
    if(is_integral(elem))
    {
      auto const i(to_i64(elem));
      return i >= 0 && static_cast<size_t>(i) < data.size();
    }
    else
    {
      return false;
    }
  }

  transient_vector_ref transient_vector::pop_in_place()
  {
    assert_active();
    if(data.empty())
    {
      throw std::runtime_error{
        "The `pop` operation on an empty `transient_vector` is not allowed."
      };
    }

    data.take(data.size() - 1);
    return runtime::detail::untagged(this);
  }

  void transient_vector::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "This `transient_vector` has already been made persistent." };
    }
  }
}
