#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  error_ptr
  analysis_invalid_case(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_def(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_fn(native_persistent_string const &message, read::source const &source);
  error_ptr analysis_invalid_fn_parameters(native_persistent_string const &message,
                                           read::source const &source);
  error_ptr analysis_invalid_recur_position(native_persistent_string const &message,
                                            read::source const &source);
  error_ptr analysis_invalid_recur_from_try(native_persistent_string const &message,
                                            read::source const &source);
  error_ptr
  analysis_invalid_recur_args(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_let(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_loop(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_if(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_quote(native_persistent_string const &message, read::source const &source);
  error_ptr analysis_invalid_var_reference(native_persistent_string const &message,
                                           read::source const &source);
  error_ptr
  analysis_invalid_throw(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_invalid_try(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_unresolved_var(native_persistent_string const &message, read::source const &source);
  error_ptr
  analysis_unresolved_symbol(native_persistent_string const &message, read::source const &source);
  error_ptr internal_analysis_failure(native_persistent_string const &message);
  error_ptr
  internal_analysis_failure(native_persistent_string const &message, read::source const &source);
}
