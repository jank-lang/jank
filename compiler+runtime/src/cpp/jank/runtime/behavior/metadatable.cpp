#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::behavior::detail
{
  object_ptr validate_meta(object_ptr const m)
  {
    if(!m)
    {
      throw std::runtime_error{ "invalid meta: nullptr" };
    }

    if(!is_map(m) && m != obj::nil::nil_const())
    {
      throw std::runtime_error{ util::format("invalid meta: {}", runtime::to_string(m)) };
    }

    return m;
  }
}
