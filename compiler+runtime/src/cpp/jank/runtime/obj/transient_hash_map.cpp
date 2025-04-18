#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/detail/native_persistent_array_map.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  transient_hash_map::transient_hash_map(runtime::detail::native_persistent_hash_map &&d)
    : data{ std::move(d).transient() }
  {
  }

  transient_hash_map::transient_hash_map(runtime::detail::native_persistent_hash_map const &d)
    : data{ d.transient() }
  {
  }

  transient_hash_map::transient_hash_map(runtime::detail::native_transient_hash_map &&d)
    : data{ std::move(d) }
  {
  }

  transient_hash_map::transient_hash_map(runtime::detail::native_persistent_array_map const &m)
  {
    for(auto const &e : m)
    {
      data.set(e.first, e.second);
    }
  }

  transient_hash_map_ref transient_hash_map::empty()
  {
    return make_box<transient_hash_map>();
  }

  bool transient_hash_map::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  void transient_hash_map::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string transient_hash_map::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string transient_hash_map::to_code_string() const
  {
    return to_string();
  }

  uhash transient_hash_map::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  usize transient_hash_map::count() const
  {
    assert_active();
    return data.size();
  }

  object_ref transient_hash_map::get(object_ref const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return jank_nil;
  }

  object_ref transient_hash_map::get(object_ref const key, object_ref const fallback) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return fallback;
  }

  object_ref transient_hash_map::get_entry(object_ref const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res)
    {
      return make_box<persistent_vector>(std::in_place, key, *res);
    }
    return jank_nil;
  }

  bool transient_hash_map::contains(object_ref const key) const
  {
    assert_active();
    return data.find(key);
  }

  transient_hash_map_ref
  transient_hash_map::assoc_in_place(object_ref const key, object_ref const val)
  {
    assert_active();
    data.set(key, val);
    return this;
  }

  transient_hash_map_ref transient_hash_map::dissoc_in_place(object_ref const key)
  {
    assert_active();
    data.erase(key);
    return this;
  }

  transient_hash_map_ref transient_hash_map::conj_in_place(object_ref const head)
  {
    assert_active();

    if(head->type == object_type::persistent_array_map
       || head->type == object_type::persistent_hash_map)
    {
      return expect_object<transient_hash_map>(runtime::merge(this, head));
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ util::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto const vec(expect_object<persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ util::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    data.set(vec->data[0], vec->data[1]);
    return this;
  }

  transient_hash_map::persistent_type_ref transient_hash_map::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_hash_map>(std::move(data).persistent());
  }

  object_ref transient_hash_map::call(object_ref const o) const
  {
    return get(o);
  }

  object_ref transient_hash_map::call(object_ref const o, object_ref const fallback) const
  {
    return get(o, fallback);
  }

  void transient_hash_map::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
