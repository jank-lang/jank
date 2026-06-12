#include <jank/codegen/api.hpp>
#include <jank/runtime/context.hpp>

jank::runtime::object_ref _jank_eval_str(char const *edn)
{
  return jank::runtime::__rt_ctx->forcefully_read_string(edn);
}
