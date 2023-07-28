#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::behavior::detail
{
  obj::map_ptr validate_meta(object_ptr const m)
  {
    if(!m)
    { throw std::runtime_error{ fmt::format("invalid meta: nullptr") }; }

    if(m->type != object_type::map)
    { throw std::runtime_error{ fmt::format("invalid meta: {}", runtime::detail::to_string(m)) }; }

    return expect_object<obj::map>(m);
  }
}
