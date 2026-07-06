#include <jank/runtime/ns.hpp>

namespace jank::runtime
{
  jtl::result<void, error_ref> ns::refer_global(object_ref const)
  {
    return ok();
  }

  jtl::result<void, error_ref> ns::rename_referred_globals(object_ref const)
  {
    return ok();
  }
}
