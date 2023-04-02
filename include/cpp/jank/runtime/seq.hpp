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

  native_bool is_nil(object_ptr o);
  native_bool is_some(object_ptr o);
  object_ptr seq(object_ptr s);
  object_ptr first(object_ptr s);
  object_ptr first(behavior::seqable_ptr const s);
  object_ptr next(object_ptr s);
  object_ptr next(behavior::seqable_ptr const s);
  object_ptr conj(object_ptr s, object_ptr o);
  object_ptr assoc(object_ptr m, object_ptr k, object_ptr v);
  object_ptr get(object_ptr o, object_ptr key);
  object_ptr get(object_ptr o, object_ptr key, object_ptr fallback);
}
