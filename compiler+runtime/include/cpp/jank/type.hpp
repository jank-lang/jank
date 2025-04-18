#pragma once

#include <map>
#include <set>
#include <string_view>
#include <unordered_map>
#include <deque>
#include <list>

#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

#include <immer/heap/gc_heap.hpp>
#include <immer/heap/heap_policy.hpp>
#include <immer/memory_policy.hpp>

#include <folly/FBVector.h>

#include <jtl/primitive.hpp>

namespace jank
{
  template <typename T>
  using native_allocator = gc_allocator<T>;
  using memory_policy = immer::memory_policy<immer::heap_policy<immer::gc_heap>,
                                             immer::no_refcount_policy,
                                             immer::default_lock_policy,
                                             immer::gc_transience_policy,
                                             false>;

  using native_real = double;
  using native_bool = bool;
  using native_hash = uint32_t;
  using native_persistent_string_view = std::string_view;

  template <typename T>
  using native_vector = folly::fbvector<T, native_allocator<T>>;
  template <typename T>
  using native_deque = std::deque<T, native_allocator<T>>;
  template <typename T>
  using native_list = std::list<T, native_allocator<T>>;
  template <typename K, typename V>
  using native_map = std::map<K, V, native_allocator<std::pair<K const, V>>>;
  template <typename T>
  using native_set = std::set<T, std::less<T>, native_allocator<T>>;

  template <typename K, typename V, typename Hash = std::hash<K>, typename Pred = std::equal_to<K>>
  using native_unordered_map
    = std::unordered_map<K, V, Hash, Pred, native_allocator<std::pair<K const, V>>>;

  /* TODO: This will leak if it's stored in a GC-tracked object. */
  using native_transient_string = std::string;
}

#include <jank/hash.hpp>

/* NOTE: jtl::immutable_string.hpp includes this file to learn about integer
 * types, but we also include it to forward our string type. Pragma once allows
 * this to work, but we need to make sure the order is right. */
#include <jtl/immutable_string.hpp>
