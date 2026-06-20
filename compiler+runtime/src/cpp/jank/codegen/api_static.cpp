#include <jank/codegen/api.hpp>
#include <jank/runtime/context.hpp>

jank::runtime::object_ref _jank_eval_str(char const * const edn)
{
  /* We cannot eval in a static runtime, so we just read. */
  return jank::runtime::__rt_ctx->forcefully_read_string(edn);
}
