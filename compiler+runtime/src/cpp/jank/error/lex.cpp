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
                      "Expected a character value here, like \\a");
  }

  error_ptr lex_invalid_number(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::lex_invalid_number, source, note);
  }

  error_ptr lex_invalid_number(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_number, message, source, "Found here");
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

  error_ptr lex_invalid_symbol(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_symbol, message, source, "Found here");
  }

  error_ptr lex_invalid_keyword(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_keyword, message, source, "Found here");
  }

  error_ptr lex_invalid_keyword(native_persistent_string const &message,
                                read::source const &source,
                                native_persistent_string const &note)
  {
    return make_error(kind::lex_invalid_keyword, message, source, note);
  }

  error_ptr lex_unterminated_string(read::source const &source)
  {
    return make_error(kind::lex_unterminated_string, source);
  }

  error_ptr
  lex_invalid_string_escape(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_invalid_string_escape, message, source, "Found here");
  }

  error_ptr
  lex_unexpected_character(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::lex_unexpected_character, message, source, "Found here");
  }

  error_ptr internal_lex_failure(read::source const &source)
  {
    return make_error(kind::internal_lex_failure, source);
  }
}
