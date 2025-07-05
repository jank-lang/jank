#include <jank/error.hpp>
#include <jank/error/aot.hpp>
#include <jank/util/fmt.hpp>

namespace jank::error
{

  error_ref aot_compilation_failure()
  {
    return make_error(kind::aot_compilation_failure, read::source::unknown);
  }

  error_ref aot_clang_executable_not_found()
  {
    return make_error(kind::aot_clang_executable_not_found, read::source::unknown);
  }

  error_ref internal_aot_failure(jtl::immutable_string const &message)
  {
    return make_error(kind::internal_aot_failure, message, read::source::unknown);
  }
}
