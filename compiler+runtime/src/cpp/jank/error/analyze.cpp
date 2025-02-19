#include <jank/error/analyze.hpp>

namespace jank::error
{
  error_ptr
  analysis_invalid_case(native_persistent_string const &message, read::source const &source)
  {
    return make_error(
      kind::analysis_invalid_case,
      message,
      source,
      note{ "Consider using the 'case' macro instead of using 'case*' directly.", source });
  }

  error_ptr
  analysis_invalid_def(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_def, message, source);
  }

  error_ptr analysis_invalid_fn(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_fn, message, source);
  }

  error_ptr analysis_invalid_fn_parameters(native_persistent_string const &message,
                                           read::source const &source)
  {
    return make_error(kind::analysis_invalid_fn_parameters, message, source);
  }

  error_ptr analysis_invalid_recur_position(native_persistent_string const &message,
                                            read::source const &source)
  {
    return make_error(kind::analysis_invalid_recur_position, message, source);
  }

  error_ptr analysis_invalid_recur_from_try(native_persistent_string const &message,
                                            read::source const &source)
  {
    return make_error(kind::analysis_invalid_recur_from_try, message, source);
  }

  error_ptr
  analysis_invalid_recur_args(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_recur_args, message, source);
  }

  error_ptr
  analysis_invalid_let(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_let, message, source);
  }

  error_ptr
  analysis_invalid_loop(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_loop, message, source);
  }

  error_ptr analysis_invalid_if(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_if, message, source);
  }

  error_ptr
  analysis_invalid_quote(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_quote, message, source);
  }

  error_ptr analysis_invalid_var_reference(native_persistent_string const &message,
                                           read::source const &source)
  {
    return make_error(kind::analysis_invalid_var_reference, message, source);
  }

  error_ptr
  analysis_invalid_throw(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_throw, message, source);
  }

  error_ptr
  analysis_invalid_try(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_invalid_try, message, source);
  }

  error_ptr
  analysis_unresolved_var(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_unresolved_var, message, source);
  }

  error_ptr
  analysis_unresolved_symbol(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::analysis_unresolved_symbol, message, source);
  }

  error_ptr internal_analysis_failure(native_persistent_string const &message)
  {
    return make_error(kind::internal_analysis_failure, message, read::source::unknown);
  }

  error_ptr
  internal_analysis_failure(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::internal_analysis_failure, message, source);
  }
}
