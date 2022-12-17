#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  object_ptr seq(object_ptr const &s);
  object_ptr mapv(object_ptr const &f, object_ptr const &seq);
  object_ptr reduce(object_ptr const &f, object_ptr const &initial, object_ptr const &seq);
  object_ptr partition_gen_minus_all(object_ptr const &n, object_ptr const &seq);
  object_ptr range(object_ptr const &start, object_ptr const &end);
  object_ptr reverse(object_ptr const &seq);
  object_ptr get(object_ptr const &o, object_ptr const &key);
  object_ptr conj(object_ptr const &o, object_ptr const &val);
  object_ptr assoc(object_ptr const &o, object_ptr const &key, object_ptr const &val);
}
