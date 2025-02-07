#include <jank/error/lex.hpp>
#include <jank/util/string_builder.hpp>

namespace jank::error
{
  error_ptr lex_unexpected_eof(read::source const &source)
  {
    return make_error(kind::lex_unexpected_eof, source, "Found here");
  }

  error_ptr lex_expecting_whitespace(read::source const &source)
  {
    return make_error(kind::lex_expecting_whitespace, source, "Found here");
  }

  error_ptr lex_invalid_unicode(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_unicode, message, source, "Found here");
  }

  error_ptr
  lex_incomplete_character(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_incomplete_character,
                      message,
                      source,
                      "Expecting a character value here, like \\a");
  }

  error_ptr lex_invalid_number(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::lex_invalid_number, source, note);
  }

  error_ptr lex_invalid_number(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_number, message, source);
  }

  error_ptr lex_invalid_number(native_persistent_string const &message,
                               read::source const &source,
                               native_persistent_string const &note)
  {
    return make_error(kind::lex_invalid_number, message, source, note);
  }

  error_ptr lex_invalid_number(native_persistent_string const &message,
                               read::source const &source,
                               note const &note)
  {
    return make_error(kind::lex_invalid_number, message, source, note);
  }

  error_ptr lex_invalid_ratio(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_ratio, message, source);
  }

  error_ptr lex_invalid_ratio(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::lex_invalid_ratio, source, note);
  }

  error_ptr lex_invalid_ratio(native_persistent_string const &message,
                              read::source const &source,
                              note const &note)
  {
    return make_error(kind::lex_invalid_ratio, message, source, note);
  }

  error_ptr lex_invalid_symbol(read::source const &source)
  {
    return make_error(kind::lex_invalid_symbol, source);
  }

  error_ptr lex_invalid_keyword(read::source const &source)
  {
    return make_error(kind::lex_invalid_keyword, source);
  }

  error_ptr lex_unterminated_string(read::source const &source)
  {
    return make_error(kind::lex_unterminated_string, source);
  }

  error_ptr lex_invalid_string_escape(read::source const &source)
  {
    return make_error(kind::lex_invalid_string_escape, source);
  }

  error_ptr lex_unexpected_character(read::source const &source)
  {
    return make_error(kind::lex_unexpected_character, source);
  }

  error_ptr internal_lex_failure(read::source const &source)
  {
    return make_error(kind::internal_lex_failure, source);
  }
}
