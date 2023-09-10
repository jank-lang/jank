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
}
