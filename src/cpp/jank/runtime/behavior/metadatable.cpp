#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::behavior
{
  obj::map_ptr metadatable::validate_meta(object_ptr const m)
  {
    if(!m)
    { throw std::runtime_error{ fmt::format("invalid meta: nullptr") }; }

    obj::map_ptr const ret{ const_cast<obj::map_ptr>(m->as_map()) };
    if(!ret)
    { throw std::runtime_error{ fmt::format("invalid meta: {}", m->to_string()) }; }

    return ret;
  }
}
