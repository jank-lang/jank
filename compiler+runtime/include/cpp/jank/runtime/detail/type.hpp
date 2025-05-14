#pragma once

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>

#include <bpptree/bpptree.hpp>
#include <bpptree/ordered.hpp>

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

namespace bpptree::detail
{
  extern template struct BppTreeSet<jank::runtime::object_ref,
                                    jank::runtime::detail::object_ref_compare>;
  extern template struct BppTreeMap<jank::runtime::object_ref,
                                    jank::runtime::object_ref,
                                    jank::runtime::detail::object_ref_compare>;
}

namespace jank::runtime::detail
{
  using native_persistent_vector = immer::vector<object_ref, memory_policy>;
  using native_transient_vector = native_persistent_vector::transient_type;

  using native_persistent_hash_set = immer::
    set<object_ref, std::hash<object_ref>, std::equal_to<jank::runtime::object_ref>, memory_policy>;
  using native_transient_hash_set = native_persistent_hash_set::transient_type;

  /* TODO: These BppTree types will leak until we get them GC allocated. */
  using native_persistent_sorted_set
    = bpptree::BppTreeSet<object_ref, object_ref_compare>::Persistent;
  using native_transient_sorted_set
    = bpptree::BppTreeSet<object_ref, object_ref_compare>::Transient;

  using native_persistent_hash_map = immer::map<object_ref,
                                                object_ref,
                                                std::hash<object_ref>,
                                                std::equal_to<jank::runtime::object_ref>,
                                                jank::memory_policy>;
  using native_transient_hash_map = native_persistent_hash_map::transient_type;

  using native_persistent_sorted_map
    = bpptree::BppTreeMap<object_ref, object_ref, object_ref_compare>::Persistent;
  using native_transient_sorted_map
    = bpptree::BppTreeMap<object_ref, object_ref, object_ref_compare>::Transient;

  /* If an object requires this in its constructor, use your runtime context to intern
   * it instead. */
  struct must_be_interned
  {
  };
}
