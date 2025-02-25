#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>

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

  persistent_array_map::persistent_array_map(object_ptr const meta, value_type &&d)
    : data{ std::move(d) }
  {
    this->meta = meta;
  }

  persistent_array_map::persistent_array_map(option<object_ptr> const &meta, value_type &&d)
    : parent_type{ meta }
    , data{ std::move(d) }
  {
  }

  object_ptr persistent_array_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return res;
    }
    return nil::nil_const();
  }

  object_ptr persistent_array_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return res;
    }
    return fallback;
  }

  object_ptr persistent_array_map::get_entry(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return make_box<persistent_vector>(std::in_place, key, res);
    }
    return nil::nil_const();
  }

  native_bool persistent_array_map::contains(object_ptr const key) const
  {
    return data.find(key);
  }

  object_ptr persistent_array_map::assoc(object_ptr const key, object_ptr const val) const
  {
    /* If we've hit the max array map size, it's time to promote to a hash map.
     *
     * Note, this currently doesn't check if the assoc is adding a new key or updating an
     * existing one, so it's possible that updating a key while at the max capacity ends up
     * promoting to a hash map.
     *
     * TODO: Benchmark if it's faster to have this behavior or to check first. */
    if(data.size() == runtime::detail::native_persistent_array_map::max_size)
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

  persistent_array_map_ptr persistent_array_map::dissoc(object_ptr const key) const
  {
    auto copy(data.clone());
    copy.erase(key);
    return make_box<persistent_array_map>(meta, std::move(copy));
  }

  object_ptr persistent_array_map::call(object_ptr const o) const
  {
    auto const found(data.find(o));
    if(!found)
    {
      return nil::nil_const();
    }
    return found;
  }

  object_ptr persistent_array_map::call(object_ptr const o, object_ptr const fallback) const
  {
    auto const found(data.find(o));
    if(!found)
    {
      return fallback;
    }
    return found;
  }

  transient_hash_map_ptr persistent_array_map::to_transient() const
  {
    /* TODO: Use a transient_array_map. */
    return make_box<transient_hash_map>(data);
  }
}
