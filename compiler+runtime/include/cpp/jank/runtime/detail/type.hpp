#pragma once

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::detail
{
  bool equal(char const lhs, object_ptr const rhs);
  native_bool equal(object_ptr const lhs, object_ptr const rhs);

  struct object_ptr_equal
  {
    static native_bool equal(object_ptr const &l, object_ptr const &r)
    {
      return detail::equal(l, r);
    }

    inline native_bool operator()(object_ptr const &l, object_ptr const &r) const
    {
      return detail::equal(l, r);
    }
  };

  using native_persistent_vector = immer::vector<object_ptr, memory_policy>;
  using native_transient_vector = native_persistent_vector::transient_type;
  using native_persistent_set
    = immer::set<object_ptr, std::hash<object_ptr>, object_ptr_equal, memory_policy>;
  using native_transient_set = native_persistent_set::transient_type;
  using native_persistent_hash_map = immer::
    map<object_ptr, object_ptr, std::hash<object_ptr>, object_ptr_equal, jank::memory_policy>;
  using native_transient_hash_map = native_persistent_hash_map::transient_type;
}
