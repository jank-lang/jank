#pragma once

#include <mutex>
#include <condition_variable>

#include <jtl/primitive.hpp>
#include <jank/type.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime::detail
{
  struct native_array_blocking_queue
  {
    native_array_blocking_queue(usize const capacity);

    object_ref take();
    bool offer(object_ref const value);

    native_vector<object_ref> buffer;
    usize head{};
    usize tail{};
    usize size{};
    usize capacity{};
    std::mutex mutex;
    std::condition_variable not_empty;
  };

  // TODO: remove this constructor function when it can be done via interop
  // instead.
  native_array_blocking_queue *create_native_array_blocking_queue(usize const capacity);
}
