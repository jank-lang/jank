#include <jank/error/codegen.hpp>

namespace jank::error
{
  error_ref codegen_internal_failure(jtl::immutable_string const &message)
  {
    return make_error(kind::codegen_internal_failure, message, read::source::unknown());
  }
}
