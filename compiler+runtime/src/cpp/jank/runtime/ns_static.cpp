#include <jank/runtime/ns.hpp>

namespace jank::runtime
{
  jtl::result<void, error_ref> ns::refer_global(object_ref const)
  {
    return ok();
  }
}
