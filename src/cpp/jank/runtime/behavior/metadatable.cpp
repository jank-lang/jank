#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::behavior
{
  void metadatable::validate_meta(object_ptr m)
  {
    if(m == nullptr || m->as_map() == nullptr)
    { throw std::runtime_error{ fmt::format("invalid meta: {}", m ? m->to_string() : "nullptr") }; }

    static_cast<void>(m);
  }
}
