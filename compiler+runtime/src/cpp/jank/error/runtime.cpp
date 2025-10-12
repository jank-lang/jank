#include <jank/error.hpp>
#include <jank/error/runtime.hpp>
#include <jank/util/fmt.hpp>

namespace jank::error
{
  error_ref runtime_module_not_found(jtl::immutable_string const &message)
  {
    return make_error(kind::runtime_module_not_found, message, read::source::unknown);
  }

  error_ref runtime_module_binary_without_source(jtl::immutable_string const &message)
  {
    return make_error(kind::runtime_module_binary_without_source, message, read::source::unknown);
  }

  error_ref runtime_unable_to_open_file(jtl::immutable_string const &message)
  {
    return make_error(kind::runtime_unable_to_open_file, message, read::source::unknown);
  }

  error_ref runtime_invalid_cpp_eval()
  {
    return make_error(kind::runtime_invalid_cpp_eval, read::source::unknown);
  }

  error_ref runtime_unable_to_load_module(jtl::immutable_string const &message)
  {
    return make_error(kind::runtime_unable_to_load_module, message, read::source::unknown);
  }

  error_ref internal_runtime_failure(jtl::immutable_string const &message)
  {
    return make_error(kind::internal_runtime_failure, message, read::source::unknown);
  }
}
