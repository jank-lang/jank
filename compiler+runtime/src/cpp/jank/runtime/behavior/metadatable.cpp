#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::behavior::detail
{
  object_ref validate_meta(object_ref const m)
  {
    if(!is_map(m) && m != jank_nil)
    {
      throw std::runtime_error{ util::format("invalid meta: {}", runtime::to_string(m)) };
    }

    return m;
  }
}
