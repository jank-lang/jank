#include <jank/runtime/obj/transient_array_map.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/detail/native_array_map.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  transient_array_map::transient_array_map(runtime::detail::native_array_map &&d)
    : data{ std::move(d) }
  {
  }

  transient_array_map::transient_array_map(runtime::detail::native_array_map const &d)
    : data{ d }
  {
  }

  transient_array_map_ref transient_array_map::empty()
  {
    return make_box<transient_array_map>();
  }

  bool transient_array_map::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  void transient_array_map::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string transient_array_map::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string transient_array_map::to_code_string() const
  {
    return to_string();
  }

  uhash transient_array_map::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  usize transient_array_map::count() const
  {
    assert_active();
    return data.size();
  }

  object_ref transient_array_map::get(object_ref const key) const
  {
    assert_active();
    return data.find(key);
  }

  object_ref transient_array_map::get(object_ref const key, object_ref const fallback) const
  {
    auto const res(get(key));
    if(res.is_some())
    {
      return res;
    }
    return fallback;
  }

  object_ref transient_array_map::get_entry(object_ref const key) const
  {
    auto const res(get(key));
    if(res.is_some())
    {
      return make_box<persistent_vector>(std::in_place, key, res);
    }
    return jank_nil;
  }

  bool transient_array_map::contains(object_ref const key) const
  {
    return get(key).is_some();
  }

  object_ref transient_array_map::assoc_in_place(object_ref const key, object_ref const value)
  {
    assert_active();
    /* If we've hit the max array map size, it's time to promote to a hash map.
     *
     * Note, this currently doesn't check if the assoc is adding a new key or updating an
     * existing one, so it's possible that updating a key while at the max capacity ends up
     * promoting to a hash map.
     *
     * TODO: Benchmark if it's faster to have this behavior or to check first. */
    /* TODO: Benchmark how much does the promotion cost, from transient array map to transient hash map? */
    if(data.size() == runtime::detail::native_array_map::max_size)
    {
      auto const promoted_map{ make_box<transient_hash_map>(data) };
      promoted_map->assoc_in_place(key, value);
      return promoted_map;
    }
    else
    {
      data.insert_or_assign(key, value);
      return this;
    }
  }

  transient_array_map_ref transient_array_map::dissoc_in_place(object_ref const key)
  {
    assert_active();
    data.erase(key);
    return this;
  }

  object_ref transient_array_map::conj_in_place(object_ref const head)
  {
    assert_active();

    if(head.is_nil())
    {
      return this;
    }

    if(is_map(head))
    {
      return runtime::merge_in_place(this, head);
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

    data.insert_or_assign(vec->data[0], vec->data[1]);

    return this;
  }

  transient_array_map::persistent_type_ref transient_array_map::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_array_map>(std::move(data));
  }

  object_ref transient_array_map::call(object_ref const o) const
  {
    return get(o);
  }

  object_ref transient_array_map::call(object_ref const o, object_ref const fallback) const
  {
    return get(o, fallback);
  }

  void transient_array_map::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
