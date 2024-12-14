#include <jank/runtime/obj/transient_sorted_set.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>

namespace jank::runtime::obj
{
  transient_sorted_set::transient_sorted_set(runtime::detail::native_persistent_sorted_set &&d)
    : data{ std::move(d).transient() }
  {
  }

  transient_sorted_set::transient_sorted_set(runtime::detail::native_persistent_sorted_set const &d)
    : data{ d.transient() }
  {
  }

  transient_sorted_set::transient_sorted_set(runtime::detail::native_transient_sorted_set &&d)
    : data{ std::move(d) }
  {
  }

  transient_sorted_set_ptr transient_sorted_set::empty()
  {
    return make_box<transient_sorted_set>();
  }

  native_bool transient_sorted_set::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  native_persistent_string transient_sorted_set::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void transient_sorted_set::to_string(fmt::memory_buffer &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "{}@{}", object_type_str(base.type), fmt::ptr(&base));
  }

  native_persistent_string transient_sorted_set::to_code_string() const
  {
    return to_string();
  }

  native_hash transient_sorted_set::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t transient_sorted_set::count() const
  {
    assert_active();
    return data.size();
  }

  transient_sorted_set_ptr transient_sorted_set::conj_in_place(object_ptr const elem)
  {
    assert_active();
    data.insert_v(elem);
    return this;
  }

  transient_sorted_set::persistent_type_ptr transient_sorted_set::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_sorted_set>(data.persistent());
  }

  object_ptr transient_sorted_set::call(object_ptr const elem)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return found.get();
    }
    return nil::nil_const();
  }

  object_ptr transient_sorted_set::call(object_ptr const elem, object_ptr const fallback)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return found.get();
    }
    return fallback;
  }

  object_ptr transient_sorted_set::get(object_ptr const elem)
  {
    return call(elem);
  }

  object_ptr transient_sorted_set::get(object_ptr const elem, object_ptr const fallback)
  {
    return call(elem, fallback);
  }

  object_ptr transient_sorted_set::get_entry(object_ptr const elem)
  {
    auto const found = call(elem);
    auto const nil(nil::nil_const());
    if(found == nil)
    {
      return nil;
    }

    return make_box<persistent_vector>(std::in_place, found, found);
  }

  native_bool transient_sorted_set::contains(object_ptr const elem) const
  {
    assert_active();
    return data.find(elem) != data.end();
  }

  transient_sorted_set_ptr transient_sorted_set::disjoin_in_place(object_ptr const elem)
  {
    assert_active();
    data.erase_key(elem);
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
