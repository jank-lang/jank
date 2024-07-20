#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::behavior::detail
{
  object_ptr validate_meta(object_ptr const m)
  {
    if(!m)
    {
      throw std::runtime_error{ fmt::format("invalid meta: nullptr") };
    }

    if(!is_map(m) && m != obj::nil::nil_const())
    {
      throw std::runtime_error{ fmt::format("invalid meta: {}", runtime::to_string(m)) };
    }

    return m;
  }
}
