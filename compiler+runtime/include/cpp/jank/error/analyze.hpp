#pragma once

#include <jank/error.hpp>

namespace cpptrace
{
  struct stacktrace;
}

namespace jank::error
{
  error_ref analyze_invalid_case(jtl::immutable_string const &message,
                                 read::source const &source,
                                 runtime::object_ptr expansion);
  error_ref analyze_invalid_def(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ptr expansion);
  error_ref analyze_invalid_def(jtl::immutable_string const &message,
                                read::source const &source,
                                jtl::immutable_string const &note,
                                runtime::object_ptr expansion);
  error_ref analyze_invalid_fn(jtl::immutable_string const &message,
                               read::source const &source,
                               runtime::object_ptr expansion);
  error_ref analyze_invalid_fn_parameters(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ptr expansion);
  error_ref analyze_invalid_fn_parameters(jtl::immutable_string const &message,
                                          read::source const &source,
                                          jtl::immutable_string const &error_note_message,
                                          runtime::object_ptr expansion);
  error_ref analyze_invalid_recur_position(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ptr expansion);
  error_ref analyze_invalid_recur_from_try(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ptr expansion);
  error_ref analyze_invalid_recur_args(jtl::immutable_string const &message,
                                       read::source const &source,
                                       runtime::object_ptr expansion);
  error_ref analyze_invalid_let(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ptr expansion);
  error_ref analyze_invalid_loop(jtl::immutable_string const &message,
                                 read::source const &source,
                                 runtime::object_ptr expansion);
  error_ref analyze_invalid_if(jtl::immutable_string const &message,
                               read::source const &source,
                               runtime::object_ptr expansion);
  error_ref analyze_invalid_if(jtl::immutable_string const &message,
                               read::source const &source,
                               jtl::immutable_string const &error_note_message,
                               runtime::object_ptr expansion);
  error_ref analyze_invalid_quote(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr expansion);
  error_ref analyze_invalid_var_reference(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ptr expansion);
  error_ref analyze_invalid_throw(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr expansion);
  error_ref analyze_invalid_try(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ptr expansion);
  error_ref analyze_invalid_try(jtl::immutable_string const &message,
                                read::source const &source,
                                note &&extra,
                                runtime::object_ptr expansion);
  error_ref analyze_unresolved_var(jtl::immutable_string const &message,
                                   read::source const &source,
                                   runtime::object_ptr expansion);
  error_ref analyze_unresolved_symbol(jtl::immutable_string const &message,
                                      read::source const &source,
                                      runtime::object_ptr expansion);
  error_ref analyze_macro_expansion_exception(std::exception const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ref analyze_macro_expansion_exception(runtime::object_ptr const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ref analyze_macro_expansion_exception(jtl::immutable_string const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ref analyze_macro_expansion_exception(error_ref e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ref
  internal_analyze_failure(jtl::immutable_string const &message, runtime::object_ptr expansion);
  error_ref internal_analyze_failure(jtl::immutable_string const &message,
                                     read::source const &source,
                                     runtime::object_ptr expansion);
}
