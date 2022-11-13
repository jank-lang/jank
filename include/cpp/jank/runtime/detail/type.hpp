#pragma once

#include <immer/memory_policy.hpp>

#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace jank::runtime::detail
{
  using memory_policy = immer::memory_policy<immer::free_list_heap_policy<immer::cpp_heap>, immer::refcount_policy, immer::default_lock_policy>;
  using integer_type = long long;
  using real_type = long double;
  using boolean_type = bool;

  template <typename T>
  using box_type = boost::intrusive_ptr<T>;
}

/* XXX: string_type.hpp includes this file to learn about integer types, but we also include it
 * to forward our string type. Pragma once allows this to work, but we need to make sure the order
 * is right. */
#include <jank/runtime/detail/string_type.hpp>

namespace jank::runtime::detail
{
  using string_type = string_type_impl<memory_policy>;
}
