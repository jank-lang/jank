#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::behavior
{
  void metadatable::validate_meta(object_ptr const &m)
  {
    assert(m != nullptr);
    assert(m->as_map() != nullptr);
  }
}
