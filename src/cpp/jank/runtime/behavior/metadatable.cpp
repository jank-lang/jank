#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::runtime::behavior::detail
{
  object_ptr validate_meta(object_ptr const m)
  {
    if(!m)
    {
      throw std::runtime_error{ fmt::format("invalid meta: nullptr") };
    }

    if(!is_map(m))
    {
      throw std::runtime_error{ fmt::format("invalid meta: {}", runtime::detail::to_string(m)) };
    }

    return m;
  }
}
