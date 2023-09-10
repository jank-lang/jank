#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime
{
  obj::persistent_hash_map::static_object(detail::native_array_map const &m, object_ptr const key, object_ptr const val)
  {
    detail::native_transient_hash_map transient;
    for(auto const &e : m)
    { transient.set(e.first, e.second); }
    transient.set(key, val);
    data = transient.persistent();
  }

  object_ptr obj::persistent_hash_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    { return *res; }
    return obj::nil::nil_const();
  }
  object_ptr obj::persistent_hash_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    { return *res; }
    return fallback;
  }

  object_ptr obj::persistent_hash_map::assoc(object_ptr const key, object_ptr const val) const
  {
    auto copy(data.set(key, val));
    return make_box<obj::persistent_hash_map>(std::move(copy));
  }
}
