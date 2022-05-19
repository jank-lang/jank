#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <immer/memory_policy.hpp>
#pragma clang diagnostic pop

#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <jank/runtime/detail/string_type.hpp>

namespace jank::runtime::detail
{
  using memory_policy = immer::memory_policy<immer::free_list_heap_policy<immer::cpp_heap>, immer::refcount_policy, immer::default_lock_policy>;
  using integer_type = int64_t;
  using real_type = double;
  using boolean_type = bool;
  using string_type = string_type_impl<memory_policy>;

  template <typename T>
  using box_type = boost::intrusive_ptr<T>;
}
