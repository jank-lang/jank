#pragma once

#include <jank/error.hpp>

namespace cpptrace
{
  struct stacktrace;
}

namespace jank::error
{
  error_ptr analyze_invalid_case(native_persistent_string const &message,
                                 read::source const &source,
                                 runtime::object_ptr expansion);
  error_ptr analyze_invalid_def(native_persistent_string const &message,
                                read::source const &source,
                                runtime::object_ptr expansion);
  error_ptr analyze_invalid_def(native_persistent_string const &message,
                                read::source const &source,
                                native_persistent_string const &note,
                                runtime::object_ptr expansion);
  error_ptr analyze_invalid_fn(native_persistent_string const &message,
                               read::source const &source,
                               runtime::object_ptr expansion);
  error_ptr analyze_invalid_fn_parameters(native_persistent_string const &message,
                                          read::source const &source,
                                          runtime::object_ptr expansion);
  error_ptr analyze_invalid_fn_parameters(native_persistent_string const &message,
                                          read::source const &source,
                                          native_persistent_string const &error_note_message,
                                          runtime::object_ptr expansion);
  error_ptr analyze_invalid_recur_position(native_persistent_string const &message,
                                           read::source const &source,
                                           runtime::object_ptr expansion);
  error_ptr analyze_invalid_recur_from_try(native_persistent_string const &message,
                                           read::source const &source,
                                           runtime::object_ptr expansion);
  error_ptr analyze_invalid_recur_args(native_persistent_string const &message,
                                       read::source const &source,
                                       runtime::object_ptr expansion);
  error_ptr analyze_invalid_let(native_persistent_string const &message,
                                read::source const &source,
                                runtime::object_ptr expansion);
  error_ptr analyze_invalid_loop(native_persistent_string const &message,
                                 read::source const &source,
                                 runtime::object_ptr expansion);
  error_ptr analyze_invalid_if(native_persistent_string const &message,
                               read::source const &source,
                               runtime::object_ptr expansion);
  error_ptr analyze_invalid_if(native_persistent_string const &message,
                               read::source const &source,
                               native_persistent_string const &error_note_message,
                               runtime::object_ptr expansion);
  error_ptr analyze_invalid_quote(native_persistent_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr expansion);
  error_ptr analyze_invalid_var_reference(native_persistent_string const &message,
                                          read::source const &source,
                                          runtime::object_ptr expansion);
  error_ptr analyze_invalid_throw(native_persistent_string const &message,
                                  read::source const &source,
                                  runtime::object_ptr expansion);
  error_ptr analyze_invalid_try(native_persistent_string const &message,
                                read::source const &source,
                                runtime::object_ptr expansion);
  error_ptr analyze_invalid_try(native_persistent_string const &message,
                                read::source const &source,
                                note &&extra,
                                runtime::object_ptr expansion);
  error_ptr analyze_unresolved_var(native_persistent_string const &message,
                                   read::source const &source,
                                   runtime::object_ptr expansion);
  error_ptr analyze_unresolved_symbol(native_persistent_string const &message,
                                      read::source const &source,
                                      runtime::object_ptr expansion);
  error_ptr analyze_macro_expansion_exception(std::exception const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ptr analyze_macro_expansion_exception(runtime::object_ptr const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ptr analyze_macro_expansion_exception(native_persistent_string const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ptr analyze_macro_expansion_exception(error_ptr e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ptr expansion);
  error_ptr
  internal_analyze_failure(native_persistent_string const &message, runtime::object_ptr expansion);
  error_ptr internal_analyze_failure(native_persistent_string const &message,
                                     read::source const &source,
                                     runtime::object_ptr expansion);
}
