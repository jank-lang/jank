#include <jank/runtime/detail/native_array_blocking_queue.hpp>

namespace jank::runtime::detail
{
  using namespace jank::runtime;

  native_array_blocking_queue::native_array_blocking_queue(usize const capacity)
    : buffer(capacity)
    , capacity(capacity)
  {
  }

  object_ref native_array_blocking_queue::take()
  {
    std::unique_lock<std::mutex> lock{ mutex };
    not_empty.wait(lock, [&] { return size > 0; });
    auto const value(buffer[head]);
    head = (head + 1) % capacity;
    --size;
    return value;
  }

  bool native_array_blocking_queue::offer(object_ref const value)
  {
    std::lock_guard<std::mutex> const lock{ mutex };
    if(size >= capacity)
    {
      return false;
    }
    buffer[tail] = value;
    tail = (tail + 1) % capacity;
    ++size;
    not_empty.notify_one();
    return true;
  }
}
