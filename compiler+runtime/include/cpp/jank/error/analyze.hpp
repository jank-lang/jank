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
                                 runtime::object_ref const expansion);
  error_ref analyze_invalid_def(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion);
  error_ref analyze_invalid_def(jtl::immutable_string const &message,
                                read::source const &source,
                                jtl::immutable_string const &note,
                                runtime::object_ref const expansion);
  error_ref analyze_invalid_fn(jtl::immutable_string const &message,
                               read::source const &source,
                               runtime::object_ref const expansion);
  error_ref analyze_invalid_fn_parameters(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ref const expansion);
  error_ref analyze_invalid_fn_parameters(jtl::immutable_string const &message,
                                          read::source const &source,
                                          jtl::immutable_string const &error_note_message,
                                          runtime::object_ref const expansion);
  error_ref analyze_invalid_recur_position(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ref const expansion);
  error_ref analyze_invalid_recur_from_try(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ref const expansion);
  error_ref analyze_invalid_recur_args(jtl::immutable_string const &message,
                                       read::source const &source,
                                       runtime::object_ref const expansion);
  error_ref analyze_invalid_let(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion);
  error_ref analyze_invalid_letfn(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ref const expansion);
  error_ref analyze_invalid_loop(jtl::immutable_string const &message,
                                 read::source const &source,
                                 runtime::object_ref const expansion);
  error_ref analyze_invalid_if(jtl::immutable_string const &message,
                               read::source const &source,
                               runtime::object_ref const expansion);
  error_ref analyze_invalid_if(jtl::immutable_string const &message,
                               read::source const &source,
                               jtl::immutable_string const &error_note_message,
                               runtime::object_ref const expansion);
  error_ref analyze_invalid_quote(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ref const expansion);
  error_ref analyze_invalid_var_reference(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ref const expansion);
  error_ref analyze_invalid_throw(jtl::immutable_string const &message,
                                  read::source const &source,
                                  runtime::object_ref const expansion);
  error_ref analyze_invalid_try(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion);
  error_ref analyze_invalid_try(jtl::immutable_string const &message,
                                read::source const &source,
                                note &&extra,
                                runtime::object_ref const expansion);
  error_ref analyze_unresolved_var(jtl::immutable_string const &message,
                                   read::source const &source,
                                   runtime::object_ref const expansion);
  error_ref analyze_unresolved_symbol(jtl::immutable_string const &message,
                                      read::source const &source,
                                      runtime::object_ref const expansion);
  error_ref analyze_macro_expansion_exception(std::exception const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_macro_expansion_exception(runtime::object_ref const e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_macro_expansion_exception(jtl::immutable_string const &e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_macro_expansion_exception(error_ref e,
                                              cpptrace::stacktrace const &trace,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_invalid_conversion(jtl::immutable_string const &message);
  error_ref analyze_invalid_cpp_operator_call(jtl::immutable_string const &message,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_constructor_call(jtl::immutable_string const &message,
                                                 read::source const &source,
                                                 runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_member_call(jtl::immutable_string const &message,
                                            read::source const &source,
                                            runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_capture(jtl::immutable_string const &message,
                                        read::source const &source,
                                        runtime::object_ref const expansion);
  error_ref analyze_mismatched_if_types(jtl::immutable_string const &message,
                                        read::source const &source,
                                        runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_function_call(jtl::immutable_string const &message,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_call(jtl::immutable_string const &message,
                                     read::source const &source,
                                     runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_conversion(jtl::immutable_string const &message,
                                           read::source const &source,
                                           runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_symbol(jtl::immutable_string const &message,
                                       read::source const &source,
                                       runtime::object_ref const expansion);
  error_ref analyze_unresolved_cpp_symbol(jtl::immutable_string const &message,
                                          read::source const &source,
                                          runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_raw(jtl::immutable_string const &message,
                                    read::source const &source,
                                    runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_type(jtl::immutable_string const &message,
                                     read::source const &source,
                                     runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_value(jtl::immutable_string const &message,
                                      read::source const &source,
                                      runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_cast(jtl::immutable_string const &message,
                                     read::source const &source,
                                     runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_box(jtl::immutable_string const &message,
                                    read::source const &source,
                                    runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_unbox(jtl::immutable_string const &message,
                                      read::source const &source,
                                      runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_new(jtl::immutable_string const &message,
                                    read::source const &source,
                                    runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_delete(jtl::immutable_string const &message,
                                       read::source const &source,
                                       runtime::object_ref const expansion);
  error_ref analyze_invalid_cpp_member_access(jtl::immutable_string const &message,
                                              read::source const &source,
                                              runtime::object_ref const expansion);
  error_ref analyze_known_issue(jtl::immutable_string const &message,
                                read::source const &source,
                                runtime::object_ref const expansion);
  error_ref internal_analyze_failure(jtl::immutable_string const &message,
                                     runtime::object_ref const expansion);
  error_ref internal_analyze_failure(jtl::immutable_string const &message,
                                     read::source const &source,
                                     runtime::object_ref const expansion);
}
