#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/to_string.hpp>
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
    return this;
  }

  transient_vector::persistent_type_ref transient_vector::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_vector>(data.persistent());
  }

  object_ref transient_vector::call(object_ref const idx) const
  {
    assert_active();
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        throw std::runtime_error{
          util::format("Index out of bound; index = {}, count = {}", i, count())
        };
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format("key must be an integer; found {}",
                                             runtime::to_string(idx)) };
    }
  }

  object_ref transient_vector::get(object_ref const idx) const
  {
    assert_active();
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return {};
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format("key must be an integer; found {}",
                                             runtime::to_string(idx)) };
    }
  }

  object_ref transient_vector::get(object_ref const idx, object_ref const fallback) const
  {
    assert_active();
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format("key must be an integer; found {}",
                                             runtime::to_string(idx)) };
    }
  }

  object_ref transient_vector::find(object_ref const idx) const
  {
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return {};
      }
      /* TODO: Map entry type? */
      return make_box<persistent_vector>(std::in_place, idx, data[i]);
    }
    else
    {
      throw std::runtime_error{ util::format("find on a vector must be an integer; found {}",
                                             runtime::to_string(idx)) };
    }
  }

  bool transient_vector::contains(object_ref const elem) const
  {
    if(elem->type == object_type::integer)
    {
      auto const i(expect_object<integer>(elem)->data);
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
      throw std::runtime_error{ "Can't pop empty vector" };
    }

    data.take(data.size() - 1);
    return this;
  }

  void transient_vector::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
