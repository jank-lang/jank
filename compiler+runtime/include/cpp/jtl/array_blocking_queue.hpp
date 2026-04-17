#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace jtl
{
  template <typename T>
  struct array_blocking_queue
  {
    std::vector<T> buffer;
    size_t head = 0, tail = 0, count = 0, capacity;
    std::mutex mutex;
    std::condition_variable not_empty;

    array_blocking_queue(size_t capacity)
      : buffer(capacity)
      , capacity(capacity)
    {
    }

    T take()
    {
      std::unique_lock<std::mutex> lock(mutex);
      not_empty.wait(lock, [&] { return count > 0; });
      T value = buffer[head];
      head = (head + 1) % capacity;
      --count;
      return value;
    }

    bool offer(T value)
    {
      std::lock_guard<std::mutex> lock(mutex);
      if(count >= capacity)
      {
        return false;
      }
      buffer[tail] = value;
      tail = (tail + 1) % capacity;
      ++count;
      not_empty.notify_one();
      return true;
    }
  };
}
