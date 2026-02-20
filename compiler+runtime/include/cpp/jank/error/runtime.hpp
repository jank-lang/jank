#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ref runtime_module_not_found(jtl::immutable_string const &message);
  error_ref runtime_module_binary_without_source(jtl::immutable_string const &message);
  error_ref runtime_unable_to_open_file(jtl::immutable_string const &message);
  error_ref runtime_invalid_cpp_eval();
  error_ref runtime_unable_to_load_module(jtl::immutable_string const &message);
  error_ref runtime_unable_to_load_module(error_ref cause);
  error_ref internal_runtime_failure(jtl::immutable_string const &message);
  error_ref
  runtime_invalid_unbox(jtl::immutable_string const &message, read::source const &unbox_source);
  error_ref runtime_invalid_unbox(jtl::immutable_string const &message,
                                  read::source const &unbox_source,
                                  read::source const &box_source);
  error_ref
  runtime_non_metadatable_value(jtl::immutable_string const &message, read::source const &source);
  error_ref runtime_invalid_referred_global_symbol(read::source const &source);
  error_ref runtime_invalid_referred_global_rename(jtl::immutable_string const &message,
                                                   read::source const &source);
  error_ref runtime_invalid_referred_global_rename(jtl::immutable_string const &message,
                                                   read::source const &rename_source,
                                                   read::source const &original_name_source);
  error_ref runtime_unsupported_behavior(runtime::object_type const type,
                                         jtl::immutable_string const &behavior,
                                         read::source const &source);
}
