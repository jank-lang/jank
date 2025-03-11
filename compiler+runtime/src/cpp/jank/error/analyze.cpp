#include <jank/error/analyze.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/meta.hpp>

namespace jank::error
{
  error_ptr analysis_invalid_case(native_persistent_string const &message,
                                  read::source const &source,
                                  native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(
      kind::analysis_invalid_case,
      message,
      source,
      note{ "Consider using the 'case' macro instead of using 'case*' directly.", source },
      macro_expansions);
  }

  error_ptr analysis_invalid_def(native_persistent_string const &message,
                                 read::source const &source,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_def, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_def(native_persistent_string const &message,
                                 read::source const &source,
                                 native_persistent_string const &note,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_def, message, source, note, macro_expansions);
  }

  error_ptr analysis_invalid_fn(native_persistent_string const &message,
                                read::source const &source,
                                native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_fn, message, source, macro_expansions);
  }

  error_ptr
  analysis_invalid_fn_parameters(native_persistent_string const &message,
                                 read::source const &source,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_fn_parameters, message, source, macro_expansions);
  }

  error_ptr
  analysis_invalid_fn_parameters(native_persistent_string const &message,
                                 read::source const &source,
                                 native_persistent_string const &error_note_message,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_fn_parameters,
                      message,
                      source,
                      error_note_message,
                      macro_expansions);
  }

  error_ptr
  analysis_invalid_recur_position(native_persistent_string const &message,
                                  read::source const &source,
                                  native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_recur_position, message, source, macro_expansions);
  }

  error_ptr
  analysis_invalid_recur_from_try(native_persistent_string const &message,
                                  read::source const &source,
                                  native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_recur_from_try, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_recur_args(native_persistent_string const &message,
                                        read::source const &source,
                                        native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_recur_args, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_let(native_persistent_string const &message,
                                 read::source const &source,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_let, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_loop(native_persistent_string const &message,
                                  read::source const &source,
                                  native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_loop, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_if(native_persistent_string const &message,
                                read::source const &source,
                                native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_if, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_if(native_persistent_string const &message,
                                read::source const &source,
                                native_persistent_string const &error_note_message,
                                native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_if,
                      message,
                      source,
                      error_note_message,
                      macro_expansions);
  }

  error_ptr analysis_invalid_quote(native_persistent_string const &message,
                                   read::source const &source,
                                   native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_quote, message, source, macro_expansions);
  }

  error_ptr
  analysis_invalid_var_reference(native_persistent_string const &message,
                                 read::source const &source,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_var_reference, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_throw(native_persistent_string const &message,
                                   read::source const &source,
                                   native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_throw, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_try(native_persistent_string const &message,
                                 read::source const &source,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_try, message, source, macro_expansions);
  }

  error_ptr analysis_invalid_try(native_persistent_string const &message,
                                 read::source const &source,
                                 note &&extra,
                                 native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_invalid_try,
                      message,
                      source,
                      std::move(extra),
                      macro_expansions);
  }

  error_ptr analysis_unresolved_var(native_persistent_string const &message,
                                    read::source const &source,
                                    native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_unresolved_var, message, source, macro_expansions);
  }

  error_ptr analysis_unresolved_symbol(native_persistent_string const &message,
                                       read::source const &source,
                                       native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::analysis_unresolved_symbol, message, source, macro_expansions);
  }

  error_ptr internal_analysis_failure(native_persistent_string const &message,
                                      native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::internal_analysis_failure,
                      message,
                      read::source::unknown,
                      macro_expansions);
  }

  error_ptr internal_analysis_failure(native_persistent_string const &message,
                                      read::source const &source,
                                      native_deque<runtime::object_ptr> const &macro_expansions)
  {
    return make_error(kind::internal_analysis_failure, message, source, macro_expansions);
  }
}
