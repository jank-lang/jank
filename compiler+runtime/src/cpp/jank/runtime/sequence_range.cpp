#include <jank/runtime/sequence_range.hpp>

namespace jank::runtime
{
  sequence_range<object_ref> make_sequence_range(object_ref const s)
  {
    return sequence_range<object_ref>{ s.is_some() ? seq(s) : s };
  }
}
