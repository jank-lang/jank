#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  namespace detail
  {
    size_t sequence_length(behavior::sequence_ptr const &s);
    size_t sequence_length(behavior::sequence_ptr const &s, size_t const max);
  }

  object_ptr seq(object_ptr s);
  object_ptr mapv(object_ptr f, object_ptr seq);
  object_ptr reduce(object_ptr f, object_ptr initial, object_ptr seq);
  object_ptr partition_gen_minus_all(object_ptr n, object_ptr seq);
  object_ptr range(object_ptr start, object_ptr end);
  object_ptr reverse(object_ptr seq);
  object_ptr get(object_ptr o, object_ptr key);
  object_ptr conj(object_ptr o, object_ptr val);
  object_ptr assoc(object_ptr o, object_ptr key, object_ptr val);
}
