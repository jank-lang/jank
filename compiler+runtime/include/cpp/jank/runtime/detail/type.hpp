#pragma once

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime::detail
{
  struct object_ref_compare
  {
    bool operator()(object_ref const l, object_ref const r) const;
  };
}

namespace immer
{
  extern template class vector<jank::runtime::object_ref, jank::memory_policy>;
  extern template class set<jank::runtime::object_ref,
                            std::hash<jank::runtime::object_ref>,
                            std::equal_to<jank::runtime::object_ref>,
                            jank::memory_policy>;
  extern template class immer::map<jank::runtime::object_ref,
                                   jank::runtime::object_ref,
                                   std::hash<jank::runtime::object_ref>,
                                   std::equal_to<jank::runtime::object_ref>,
                                   jank::memory_policy>;
}

namespace jank::runtime::detail
{
  using native_persistent_vector = immer::vector<object_ref, memory_policy>;
  using native_transient_vector = native_persistent_vector::transient_type;

  using native_persistent_hash_set = immer::
    set<object_ref, std::hash<object_ref>, std::equal_to<jank::runtime::object_ref>, memory_policy>;
  using native_transient_hash_set = native_persistent_hash_set::transient_type;

  /* TODO: Bring in proper immutable sorted maps/sets. */
  using native_persistent_sorted_set
    = std::set<object_ref, object_ref_compare, native_allocator<object_ref>>;
  using native_transient_sorted_set = native_persistent_sorted_set;

  using native_persistent_hash_map = immer::map<object_ref,
                                                object_ref,
                                                std::hash<object_ref>,
                                                std::equal_to<jank::runtime::object_ref>,
                                                jank::memory_policy>;
  using native_transient_hash_map = native_persistent_hash_map::transient_type;

  using native_persistent_sorted_map
    = std::map<object_ref,
               object_ref,
               object_ref_compare,
               native_allocator<std::pair<object_ref const, object_ref>>>;
  using native_transient_sorted_map = native_persistent_sorted_map;

  /* If an object requires this in its constructor, use your runtime context to intern
   * it instead. */
  struct must_be_interned
  {
  };
}
