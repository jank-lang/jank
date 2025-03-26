#include <jank/runtime/obj/transient_hash_set.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  transient_hash_set::transient_hash_set(runtime::detail::native_persistent_hash_set &&d)
    : data{ std::move(d).transient() }
  {
  }

  transient_hash_set::transient_hash_set(runtime::detail::native_persistent_hash_set const &d)
    : data{ d.transient() }
  {
  }

  transient_hash_set::transient_hash_set(runtime::detail::native_transient_hash_set &&d)
    : data{ std::move(d) }
  {
  }

  transient_hash_set_ptr transient_hash_set::empty()
  {
    return make_box<transient_hash_set>();
  }

  native_bool transient_hash_set::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  jtl::immutable_string transient_hash_set::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void transient_hash_set::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string transient_hash_set::to_code_string() const
  {
    return to_string();
  }

  native_hash transient_hash_set::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t transient_hash_set::count() const
  {
    assert_active();
    return data.size();
  }

  transient_hash_set_ptr transient_hash_set::conj_in_place(object_ptr const elem)
  {
    assert_active();
    data.insert(elem);
    return this;
  }

  transient_hash_set::persistent_type_ptr transient_hash_set::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_hash_set>(data.persistent());
  }

  object_ptr transient_hash_set::call(object_ptr const elem) const
  {
    assert_active();
    auto const found(data.find(elem));
    if(!found)
    {
      return nil::nil_const();
    }
    return *found;
  }

  object_ptr transient_hash_set::call(object_ptr const elem, object_ptr const fallback) const
  {
    assert_active();
    auto const found(data.find(elem));
    if(!found)
    {
      return fallback;
    }
    return *found;
  }

  object_ptr transient_hash_set::get(object_ptr const elem) const
  {
    return call(elem);
  }

  object_ptr transient_hash_set::get(object_ptr const elem, object_ptr const fallback) const
  {
    return call(elem, fallback);
  }

  object_ptr transient_hash_set::get_entry(object_ptr const elem) const
  {
    auto const found = call(elem);
    auto const nil(nil::nil_const());
    if(found == nil)
    {
      return nil;
    }

    return make_box<persistent_vector>(std::in_place, found, found);
  }

  native_bool transient_hash_set::contains(object_ptr const elem) const
  {
    assert_active();
    return data.find(elem);
  }

  transient_hash_set_ptr transient_hash_set::disjoin_in_place(object_ptr const elem)
  {
    assert_active();
    data.erase(elem);
    return this;
  }

  void transient_hash_set::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
