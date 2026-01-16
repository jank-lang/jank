#include <jank/error/codegen.hpp>

namespace jank::error
{
  error_ref internal_codegen_failure(jtl::immutable_string const &message)
  {
    return make_error(kind::internal_codegen_failure, message, read::source::unknown());
  }
}
