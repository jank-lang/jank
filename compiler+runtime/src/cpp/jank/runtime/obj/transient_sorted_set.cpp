#include <jank/runtime/obj/transient_sorted_set.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/util/fmt.hpp>

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

  transient_sorted_set_ref transient_sorted_set::empty()
  {
    return make_box<transient_sorted_set>();
  }

  bool transient_sorted_set::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  jtl::immutable_string transient_sorted_set::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void transient_sorted_set::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string transient_sorted_set::to_code_string() const
  {
    return to_string();
  }

  uhash transient_sorted_set::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  usize transient_sorted_set::count() const
  {
    assert_active();
    return data.size();
  }

  transient_sorted_set_ref transient_sorted_set::conj_in_place(object_ref const elem)
  {
    assert_active();
    data.insert_v(elem);
    return this;
  }

  transient_sorted_set::persistent_type_ref transient_sorted_set::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_sorted_set>(data.persistent());
  }

  object_ref transient_sorted_set::call(object_ref const elem)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return found.get();
    }
    return jank_nil;
  }

  object_ref transient_sorted_set::call(object_ref const elem, object_ref const fallback)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return found.get();
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
    auto const found = call(elem);
    auto const nil(jank_nil);
    if(found == nil)
    {
      return nil;
    }

    return make_box<persistent_vector>(std::in_place, found, found);
  }

  bool transient_sorted_set::contains(object_ref const elem) const
  {
    assert_active();
    return data.find(elem) != data.end();
  }

  transient_sorted_set_ref transient_sorted_set::disjoin_in_place(object_ref const elem)
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
