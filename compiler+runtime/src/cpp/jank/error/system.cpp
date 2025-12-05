#include <jank/error/system.hpp>
#include <jank/util/fmt.hpp>

namespace jank::error
{
  error_ref system_clang_executable_not_found()
  {
    return make_error(kind::system_clang_executable_not_found, read::source::unknown());
  }

  error_ref system_failure(jtl::immutable_string const &message)
  {
    return make_error(kind::system_failure, message, read::source::unknown());
  }
}
