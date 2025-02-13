#pragma once

#include <jank/error.hpp>
#include <jank/read/lex.hpp>

namespace jank::error
{
  error_ptr lex_unexpected_eof(read::source const &source);
  error_ptr lex_expecting_whitespace(read::source const &source);
  error_ptr
  lex_invalid_unicode(native_persistent_string const &message, read::source const &source);
  error_ptr
  lex_incomplete_character(native_persistent_string const &message, read::source const &source);
  error_ptr lex_invalid_number(read::source const &source, native_persistent_string const &note);
  error_ptr lex_invalid_number(native_persistent_string const &message, read::source const &source);
  error_ptr lex_invalid_number(native_persistent_string const &message,
                               read::source const &source,
                               native_persistent_string const &note);
  error_ptr lex_invalid_number(native_persistent_string const &message,
                               read::source const &source,
                               note const &note);
  error_ptr lex_invalid_ratio(native_persistent_string const &message, read::source const &source);
  error_ptr lex_invalid_ratio(read::source const &source, native_persistent_string const &note);
  error_ptr lex_invalid_ratio(native_persistent_string const &message,
                              read::source const &source,
                              note const &note);
  error_ptr lex_invalid_symbol(native_persistent_string const &message, read::source const &source);
  error_ptr
  lex_invalid_keyword(native_persistent_string const &message, read::source const &source);
  error_ptr lex_invalid_keyword(native_persistent_string const &message,
                                read::source const &source,
                                native_persistent_string const &note);
  error_ptr lex_unterminated_string(read::source const &source);
  error_ptr
  lex_invalid_string_escape(native_persistent_string const &message, read::source const &source);
  error_ptr
  lex_unexpected_character(native_persistent_string const &message, read::source const &source);
  error_ptr internal_lex_failure(read::source const &source);
}
