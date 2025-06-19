#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  persistent_array_map::persistent_array_map(value_type &&d)
    : data{ std::move(d) }
  {
  }

  persistent_array_map::persistent_array_map(value_type const &d)
    : data{ d }
  {
  }

  persistent_array_map::persistent_array_map(jtl::option<object_ref> const &meta, value_type &&d)
    : parent_type{ meta }
    , data{ std::move(d) }
  {
  }

  object_ref persistent_array_map::get(object_ref const key) const
  {
    auto const res(data.find(key));
    return res;
  }

  object_ref persistent_array_map::get(object_ref const key, object_ref const fallback) const
  {
    auto const res(data.find(key));
    if(res.is_some())
    {
      return res;
    }
    return fallback;
  }

  object_ref persistent_array_map::get_entry(object_ref const key) const
  {
    auto const res(data.find(key));
    if(res.is_some())
    {
      return make_box<persistent_vector>(std::in_place, key, res);
    }
    return jank_nil;
  }

  bool persistent_array_map::contains(object_ref const key) const
  {
    return data.find(key).is_some();
  }

  object_ref persistent_array_map::assoc(object_ref const key, object_ref const val) const
  {
    /* If we've hit the max array map size, it's time to promote to a hash map.
     *
     * Note, this currently doesn't check if the assoc is adding a new key or updating an
     * existing one, so it's possible that updating a key while at the max capacity ends up
     * promoting to a hash map.
     *
     * TODO: Benchmark if it's faster to have this behavior or to check first. */
    if(data.size() == runtime::detail::native_array_map::max_size)
    {
      return make_box<persistent_hash_map>(meta, data, key, val);
    }
    else
    {
      auto copy(data.clone());
      copy.insert_or_assign(key, val);
      return make_box<persistent_array_map>(meta, std::move(copy));
    }
  }

  persistent_array_map_ref persistent_array_map::dissoc(object_ref const key) const
  {
    auto copy(data.clone());
    copy.erase(key);
    return make_box<persistent_array_map>(meta, std::move(copy));
  }

  object_ref persistent_array_map::call(object_ref const o) const
  {
    auto const found(data.find(o));
    return found;
  }

  object_ref persistent_array_map::call(object_ref const o, object_ref const fallback) const
  {
    auto const found(data.find(o));
    if(found.is_nil())
    {
      return fallback;
    }
    return found;
  }

  transient_hash_map_ref persistent_array_map::to_transient() const
  {
    /* TODO: Use a transient_array_map. */
    return make_box<transient_hash_map>(data);
  }
}
