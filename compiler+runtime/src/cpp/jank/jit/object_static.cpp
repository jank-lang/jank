#include <jank/jit/object.hpp>

namespace jank::jit
{
  void register_loaded_object(uptr const, loaded_object const &)
  {
  }

  jtl::option<materialized_object_frame> find_materialized_object_frame(uptr const)
  {
    return none;
  }

  cpptrace::stacktrace_frame resolve_materialized_object_frame(cpptrace::stacktrace_frame frame)
  {
    return frame;
  }
}
