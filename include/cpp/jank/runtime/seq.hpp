#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  namespace detail
  {
    size_t sequence_length(behavior::sequence_ptr const s);
    size_t sequence_length(behavior::sequence_ptr const s, size_t const max);
    native_string to_string(behavior::sequence_ptr const s);
    void to_string(behavior::sequence_ptr const s, fmt::memory_buffer &buff);
  }

  object_ptr seq(object_ptr s);
  object_ptr get(object_ptr o, object_ptr key);
  object_ptr get(object_ptr o, object_ptr key, object_ptr fallback);
}
