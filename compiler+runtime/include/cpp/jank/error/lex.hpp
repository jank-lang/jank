#pragma once

#include <jank/error.hpp>
#include <jank/read/lex.hpp>

namespace jank::error
{
  error_ref lex_unexpected_eof(read::source const &source);
  error_ref lex_expecting_whitespace(read::source const &source);
  error_ref lex_invalid_unicode(jtl::immutable_string const &message, read::source const &source);
  error_ref
  lex_incomplete_character(jtl::immutable_string const &message, read::source const &source);
  error_ref lex_invalid_number(read::source const &source, jtl::immutable_string const &note);
  error_ref lex_invalid_number(jtl::immutable_string const &message, read::source const &source);
  error_ref lex_invalid_number(jtl::immutable_string const &message,
                               read::source const &source,
                               jtl::immutable_string const &note);
  error_ref lex_invalid_number(jtl::immutable_string const &message,
                               read::source const &source,
                               note const &note);
  error_ref lex_invalid_ratio(jtl::immutable_string const &message, read::source const &source);
  error_ref lex_invalid_ratio(read::source const &source, jtl::immutable_string const &note);
  error_ref lex_invalid_ratio(jtl::immutable_string const &message,
                              read::source const &source,
                              note const &note);
  error_ref lex_invalid_symbol(jtl::immutable_string const &message, read::source const &source);
  error_ref lex_invalid_keyword(jtl::immutable_string const &message, read::source const &source);
  error_ref lex_invalid_keyword(jtl::immutable_string const &message,
                                read::source const &source,
                                jtl::immutable_string const &note);
  error_ref lex_unterminated_string(read::source const &source);
  error_ref
  lex_invalid_string_escape(jtl::immutable_string const &message, read::source const &source);
  error_ref
  lex_unexpected_character(jtl::immutable_string const &message, read::source const &source);
  error_ref internal_lex_failure(read::source const &source);
}
