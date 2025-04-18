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
  error_ref analyze_invalid_case(jtl::immutable_string const &message,
                                 read::source const &source,
                                 runtime::object_ref const expansion)
  {
    return make_error(
      kind::analyze_invalid_case,
      message,
      source,
      note{ "Consider using the 'case' macro instead of using 'case*' directly.", source },
      expansion);
  }

  error_ref analyze_invalid_def(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_def, message, source, expansion);
  }

  error_ref analyze_invalid_def(jtl::immutable_string const &message,
                                read::source const &source,
                                jtl::immutable_string const &note,
                                runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_def, message, source, note, expansion);
  }

  error_ref analyze_invalid_fn(jtl::immutable_string const &message,
                               read::source const &source,
                               runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_fn, message, source, expansion);
  }

  error_ref analyze_invalid_fn_parameters(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_fn_parameters, message, source, expansion);
  }

  error_ref analyze_invalid_fn_parameters(jtl::immutable_string const &message,
                                          read::source const &source,
                                          jtl::immutable_string const &error_note_message,
                                          runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_fn_parameters,
                      message,
                      source,
                      error_note_message,
                      expansion);
  }

  error_ref analyze_invalid_recur_position(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_recur_position, message, source, expansion);
  }

  error_ref analyze_invalid_recur_from_try(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_recur_from_try, message, source, expansion);
  }

  error_ref analyze_invalid_recur_args(jtl::immutable_string const &message,
                                       read::source const &source,
                                       runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_recur_args, message, source, expansion);
  }

  error_ref analyze_invalid_let(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_let, message, source, expansion);
  }

  error_ref analyze_invalid_letfn(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_letfn, message, source, expansion);
  }

  error_ref analyze_invalid_loop(jtl::immutable_string const &message,
                                 read::source const &source,
                                 runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_loop, message, source, expansion);
  }

  error_ref analyze_invalid_if(jtl::immutable_string const &message,
                               read::source const &source,
                               runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_if, message, source, expansion);
  }

  error_ref analyze_invalid_if(jtl::immutable_string const &message,
                               read::source const &source,
                               jtl::immutable_string const &error_note_message,
                               runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_if, message, source, error_note_message, expansion);
  }

  error_ref analyze_invalid_quote(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_quote, message, source, expansion);
  }

  error_ref analyze_invalid_var_reference(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_var_reference, message, source, expansion);
  }

  error_ref analyze_invalid_throw(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_throw, message, source, expansion);
  }

  error_ref analyze_invalid_try(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_try, message, source, expansion);
  }

  error_ref analyze_invalid_try(jtl::immutable_string const &message,
                                read::source const &source,
                                note &&extra,
                                runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_invalid_try, message, source, std::move(extra), expansion);
  }

  error_ref analyze_unresolved_var(jtl::immutable_string const &message,
                                   read::source const &source,
                                   runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_unresolved_var, message, source, expansion);
  }

  error_ref analyze_unresolved_symbol(jtl::immutable_string const &message,
                                      read::source const &source,
                                      runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_unresolved_symbol, message, source, expansion);
  }

  error_ref analyze_macro_expansion_exception(std::exception const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      e.what(),
                      source,
                      expansion,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ref analyze_macro_expansion_exception(runtime::object_ref const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      e->type == runtime::object_type::persistent_string
                        ? runtime::to_string(e)
                        : runtime::to_code_string(e),
                      source,
                      expansion,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ref analyze_macro_expansion_exception(jtl::immutable_string const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      e,
                      source,
                      expansion,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ref analyze_macro_expansion_exception(error_ref const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion)
  {
    return make_error(kind::analyze_macro_expansion_exception,
                      /* TODO: Macro name. */
                      "Uncaught exception while expanding macro.",
                      source,
                      expansion,
                      e,
                      std::make_unique<cpptrace::stacktrace>(trace));
  }

  error_ref internal_analyze_failure(jtl::immutable_string const &message,
                                     runtime::object_ref const expansion)
  {
    return make_error(kind::internal_analyze_failure, message, read::source::unknown, expansion);
  }

  error_ref internal_analyze_failure(jtl::immutable_string const &message,
                                     read::source const &source,
                                     runtime::object_ref const expansion)
  {
    return make_error(kind::internal_analyze_failure, message, source, expansion);
  }
}
