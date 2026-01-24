#include <jank/runtime/obj/transient_sorted_set.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  transient_sorted_set::transient_sorted_set()
    : object{ obj_type }
  {
  }

  transient_sorted_set::transient_sorted_set(runtime::detail::native_persistent_sorted_set const &d)
    : object{ obj_type }
    , data{ d }
  {
  }

  transient_sorted_set::transient_sorted_set(runtime::detail::native_persistent_sorted_set &&d)
    : object{ obj_type }
    , data{ std::move(d) }
  {
  }

  transient_sorted_set_ref transient_sorted_set::empty()
  {
    return make_box<transient_sorted_set>();
  }

  usize transient_sorted_set::count() const
  {
    assert_active();
    return data.size();
  }

  transient_sorted_set_ref transient_sorted_set::conj_in_place(object_ref const elem)
  {
    assert_active();
    data.insert(elem);
    return this;
  }

  transient_sorted_set::persistent_type_ref transient_sorted_set::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_sorted_set>(data);
  }

  object_ref transient_sorted_set::call(object_ref const elem)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return *found;
    }
    return jank_nil();
  }

  object_ref transient_sorted_set::call(object_ref const elem, object_ref const fallback)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return *found;
    }
    return fallback;
  }

  object_ref transient_sorted_set::get(object_ref const elem)
  {
    return call(elem);
  }

  object_ref transient_sorted_set::get(object_ref const elem, object_ref const fallback)
  {
    return call(elem, fallback);
  }

  object_ref transient_sorted_set::get_entry(object_ref const elem)
  {
    auto found{ call(elem) };
    if(found == jank_nil())
    {
      return found;
    }

    return make_box<persistent_vector>(std::in_place, found, found);
  }

  bool transient_sorted_set::contains(object_ref const elem) const
  {
    assert_active();
    return data.contains(elem);
  }

  transient_sorted_set_ref transient_sorted_set::disjoin_in_place(object_ref const elem)
  {
    assert_active();
    data.erase(elem);
    return this;
  }

  void transient_sorted_set::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
