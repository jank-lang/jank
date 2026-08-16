#include <jank/jit/object.hpp>

namespace jank::jit
{
  void register_loaded_object(uptr const, loaded_object const &)
  {
  }

  cpptrace::stacktrace_frame resolve_materialized_object_frame(cpptrace::stacktrace_frame frame)
  {
    return frame;
  }
}
