#include <cpptrace/cpptrace.hpp>

#include <jank/error/analyze.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  error_ptr analyze_invalid_case(native_persistent_string const &message,
                                 read::source const &source,
                                 runtime::object_ptr const expansion)
  {
    return make_error(
      kind::analyze_invalid_case,
      message,
      source,
      note{ "Consider using the 'case' macro instead of using 'case*' directly.", source },
      expansion);
  }

  error_ptr analyze_invalid_def(native_persistent_string const &message,
                                read::source const &source,
                                runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_def, message, source, expansion);
  }

  error_ptr analyze_invalid_def(native_persistent_string const &message,
                                read::source const &source,
                                native_persistent_string const &note,
                                runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_def, message, source, note, expansion);
  }

  error_ptr analyze_invalid_fn(native_persistent_string const &message,
                               read::source const &source,
                               runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_fn, message, source, expansion);
  }

  error_ptr analyze_invalid_fn_parameters(native_persistent_string const &message,
                                          read::source const &source,
                                          runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_fn_parameters, message, source, expansion);
  }

  error_ptr analyze_invalid_fn_parameters(native_persistent_string const &message,
                                          read::source const &source,
                                          native_persistent_string const &error_note_message,
                                          runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_fn_parameters,
                      message,
                      source,
                      error_note_message,
                      expansion);
  }

  error_ptr analyze_invalid_recur_position(native_persistent_string const &message,
                                           read::source const &source,
                                           runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_recur_position, message, source, expansion);
  }

  error_ptr analyze_invalid_recur_from_try(native_persistent_string const &message,
                                           read::source const &source,
                                           runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_recur_from_try, message, source, expansion);
  }

  error_ptr analyze_invalid_recur_args(native_persistent_string const &message,
                                       read::source const &source,
                                       runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_recur_args, message, source, expansion);
  }

  error_ptr analyze_invalid_let(native_persistent_string const &message,
                                read::source const &source,
                                runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_let, message, source, expansion);
  }

  error_ptr analyze_invalid_letfn(native_persistent_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_letfn, message, source, expansion);
  }

  error_ptr analyze_invalid_loop(native_persistent_string const &message,
                                 read::source const &source,
                                 runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_loop, message, source, expansion);
  }

  error_ptr analyze_invalid_if(native_persistent_string const &message,
                               read::source const &source,
                               runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_if, message, source, expansion);
  }

  error_ptr analyze_invalid_if(native_persistent_string const &message,
                               read::source const &source,
                               native_persistent_string const &error_note_message,
                               runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_if, message, source, error_note_message, expansion);
  }

  error_ptr analyze_invalid_quote(native_persistent_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_quote, message, source, expansion);
  }

  error_ptr analyze_invalid_var_reference(native_persistent_string const &message,
                                          read::source const &source,
                                          runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_var_reference, message, source, expansion);
  }

  error_ptr analyze_invalid_throw(native_persistent_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_throw, message, source, expansion);
  }

  error_ptr analyze_invalid_try(native_persistent_string const &message,
                                read::source const &source,
                                runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_try, message, source, expansion);
  }

  error_ptr analyze_invalid_try(native_persistent_string const &message,
                                read::source const &source,
                                note &&extra,
                                runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_invalid_try, message, source, std::move(extra), expansion);
  }

  error_ptr analyze_unresolved_var(native_persistent_string const &message,
                                   read::source const &source,
                                   runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_unresolved_var, message, source, expansion);
  }

  error_ptr analyze_unresolved_symbol(native_persistent_string const &message,
                                      read::source const &source,
                                      runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_unresolved_symbol, message, source, expansion);
  }

  error_ptr analyze_macro_expansion_exception(std::exception const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      e.what(),
                      source,
                      expansion,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ptr analyze_macro_expansion_exception(runtime::object_ptr const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      e->type == runtime::object_type::persistent_string
                        ? runtime::to_string(e)
                        : runtime::to_code_string(e),
                      source,
                      expansion,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ptr analyze_macro_expansion_exception(native_persistent_string const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      e,
                      source,
                      expansion,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ptr analyze_macro_expansion_exception(error_ptr const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      /* TODO: Macro name. */
                      "Uncaught exception while expanding macro.",
                      source,
                      expansion,
                      e,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ptr internal_analyze_failure(native_persistent_string const &message,
                                     runtime::object_ptr const expansion)
  {
    return make_error(kind::internal_analyze_failure, message, read::source::unknown, expansion);
  }

  error_ptr internal_analyze_failure(native_persistent_string const &message,
                                     read::source const &source,
                                     runtime::object_ptr const expansion)
  {
    return make_error(kind::internal_analyze_failure, message, source, expansion);
  }
}
