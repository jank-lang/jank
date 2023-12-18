#pragma once

#include <map>
#include <set>
#include <string_view>

#include <folly/FBVector.h>

namespace jank
{
  template <typename T>
  using native_allocator = gc_allocator<T>;
  using memory_policy = immer::memory_policy
  <
    immer::heap_policy<immer::gc_heap>,
    immer::no_refcount_policy,
    immer::default_lock_policy,
    immer::gc_transience_policy,
    false
  >;

  using native_integer = long long;
  using native_real = long double;
  using native_bool = bool;
  using native_string_view = std::string_view;

  template <typename T>
  using native_vector = folly::fbvector<T, native_allocator<T>>;
  template <typename K, typename V>
  using native_map = std::map<K, V, native_allocator<std::pair<K const, V>>>;
  template <typename T>
  using native_set = std::set<T, std::less<T>, native_allocator<T>>;

  /* TODO: Try out unordered_flat_map once vcpkg has boost 1.81.0. */
  template
  <
    typename K,
    typename V,
    typename Hash = std::hash<K>,
    typename Pred = std::equal_to<K>
  >
  using native_unordered_map = boost::unordered_map
  <K, V, Hash, Pred, native_allocator<std::pair<K const, V>>>;
}

/* TODO: Folly strings leak memory, since they're not using the GC and the GC isn't
 * running destructors. */

/* XXX: native_string.hpp includes this file to learn about integer types, but we also include it
 * to forward our string type. Pragma once allows this to work, but we need to make sure the order
 * is right. */
#include <jank/native_string.hpp>
