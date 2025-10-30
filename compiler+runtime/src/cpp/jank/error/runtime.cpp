#include <jank/error/runtime.hpp>

namespace jank::error
{
  error_ref
  runtime_invalid_unbox(jtl::immutable_string const &message, read::source const &unbox_source)
  {
    return make_error(kind::runtime_invalid_unbox, message, unbox_source);
  }

  error_ref runtime_invalid_unbox(jtl::immutable_string const &message,
                                  read::source const &unbox_source,
                                  read::source const &box_source)
  {
    return make_error(kind::runtime_invalid_unbox,
                      message,
                      unbox_source,
                      native_vector<note>{
                        error::note{ "Unboxed here.", unbox_source },
                        error::note{ "Boxed here.", box_source, error::note::kind::info }
    });
  }

  error_ref
  runtime_non_metadatable_value(jtl::immutable_string const &message, read::source const &source)
  {
    return make_error(kind::runtime_non_metadatable_value, message, source);
  }
}
