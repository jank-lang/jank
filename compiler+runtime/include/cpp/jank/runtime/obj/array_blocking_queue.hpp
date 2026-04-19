#pragma once

#include <mutex>
#include <condition_variable>

#include <jank/type.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using namespace jank::runtime;

  struct array_blocking_queue
  {
    jank::native_vector<object_ref> buffer;
    size_t head{};
    size_t tail{};
    size_t size{};
    size_t capacity{};
    std::mutex mutex;
    std::condition_variable not_empty;

    array_blocking_queue(size_t const capacity)
      : buffer(capacity)
      , capacity(capacity)
    {
    }

    object_ref take()
    {
      std::unique_lock<std::mutex> lock(mutex);
      not_empty.wait(lock, [&] { return size > 0; });
      auto const value(buffer[head]);
      head = (head + 1) % capacity;
      --size;
      return value;
    }

    bool offer(object_ref const value)
    {
      std::lock_guard<std::mutex> lock(mutex);
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
  };
}
