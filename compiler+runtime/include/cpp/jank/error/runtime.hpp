#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref
  runtime_invalid_unbox(jtl::immutable_string const &message, read::source const &unbox_source);
  error_ref runtime_invalid_unbox(jtl::immutable_string const &message,
                                  read::source const &unbox_source,
                                  read::source const &box_source);
  error_ref
  runtime_non_metadatable_value(jtl::immutable_string const &message, read::source const &source);
}
