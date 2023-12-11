#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime
{
  object_ptr obj::persistent_array_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    { return res; }
    return obj::nil::nil_const();
  }
  object_ptr obj::persistent_array_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    { return res; }
    return fallback;
  }

  object_ptr obj::persistent_array_map::get_entry(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    { return make_box<obj::vector>(key, res); }
    return obj::nil::nil_const();
  }

  native_bool obj::persistent_array_map::contains(object_ptr const key) const
  { return data.find(key); }

  object_ptr obj::persistent_array_map::assoc(object_ptr const key, object_ptr const val) const
  {
    /* If we've hit the max array map size, it's time to promote to a hash map.
     *
     * Note, this currently doesn't check if the assoc is adding a new key or updating an
     * existing one, so it's possible that updating a key while at the max capacity ends up
     * promoting to a hash map.
     *
     * TODO: Benchmark if it's faster to have this behavior or to check first. */
    if(data.size() == detail::native_array_map::max_size)
    { return make_box<obj::persistent_hash_map>(data, key, val); }
    else
    {
      auto copy(data.clone());
      copy.insert_or_assign(key, val);
      return make_box<obj::persistent_array_map>(std::move(copy));
    }
  }

  obj::persistent_array_map_ptr obj::persistent_array_map::cons(object_ptr const head) const
  {
    if(head->type != object_type::vector)
    { throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::detail::to_string(head)) }; }

    auto const vec(expect_object<obj::vector>(head));
    if(vec->count() != 2)
    { throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::detail::to_string(head)) }; }

    auto copy(data.clone());
    copy.insert_or_assign(vec->data[0], vec->data[1]);
    return make_box<obj::persistent_array_map>(std::move(copy));
  }

  object_ptr obj::persistent_array_map::call(object_ptr const o) const
  {
    auto const found(data.find(o));
    if(!found)
    { return obj::nil::nil_const(); }
    return found;
  }

  object_ptr obj::persistent_array_map::call(object_ptr const o, object_ptr const fallback) const
  {
    auto const found(data.find(o));
    if(!found)
    { return fallback; }
    return found;
  }
}
