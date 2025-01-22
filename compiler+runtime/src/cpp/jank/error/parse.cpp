#include <jank/error/parse.hpp>
#include <jank/read/lex.hpp>
#include <jank/util/string_builder.hpp>

namespace jank::error
{
  static constexpr char char_for_token_kind(read::lex::token_kind const kind)
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch(kind)
    {
      case read::lex::token_kind::open_paren:
        return '(';
      case read::lex::token_kind::close_paren:
        return ')';
      case read::lex::token_kind::open_square_bracket:
        return '[';
      case read::lex::token_kind::close_square_bracket:
        return ']';
      case read::lex::token_kind::open_curly_bracket:
        return '{';
      case read::lex::token_kind::close_curly_bracket:
        return '}';
    }
#pragma clang diagnostic pop
    return '?';
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wreturn-type"

  error_ptr parse_invalid_unicode(read::source const &source, note const &error)
  {
  }

  error_ptr parse_invalid_character(read::source const &source)
  {
  }

  error_ptr parse_unexpected_closing_character(read::lex::token const &token)
  {
    util::string_builder sb;
    return make_error(
      kind::parse_unexpected_closing_character,
      sb("Unexpected closing character '")(char_for_token_kind(token.kind))("'").release(),
      token.start,
      "This is unexpected, since it has no matching open character");
  }

  error_ptr parse_unterminated_list(read::source const &source)
  {
  }

  error_ptr parse_unterminated_vector(read::source const &source)
  {
  }

  error_ptr parse_unterminated_map(read::source const &source)
  {
  }

  error_ptr parse_unterminated_set(read::source const &source)
  {
  }

  error_ptr
  parse_odd_entries_in_map(read::source const &map_source, read::source const &last_key_source)
  {
  }

  error_ptr parse_invalid_quote(native_persistent_string const &message, read::source const &source)
  {
  }

  error_ptr parse_invalid_meta_hint_value(read::source const &source)
  {
  }

  error_ptr parse_invalid_meta_hint_target(native_persistent_string const &message,
                                           read::source const &source)
  {
  }

  error_ptr parse_unsupported_reader_macro(read::source const &source)
  {
  }

  error_ptr
  parse_nested_shorthand_function(read::source const &source, note const &parent_fn_source)
  {
  }

  error_ptr parse_invalid_shorthand_function(read::source const &source)
  {
  }

  error_ptr parse_invalid_shorthand_function_parameter(read::source const &source)
  {
  }

  error_ptr parse_invalid_reader_var(read::source const &source)
  {
  }

  error_ptr parse_invalid_reader_comment(read::source const &source)
  {
  }

  error_ptr parse_invalid_reader_conditional(read::source const &source)
  {
  }

  error_ptr parse_invalid_reader_splice(read::source const &source)
  {
  }

  error_ptr parse_invalid_reader_gensym(read::source const &source)
  {
  }

  error_ptr parse_invalid_syntax_quote(read::source const &source)
  {
  }

  error_ptr parse_invalid_syntax_unquote(read::source const &source)
  {
  }

  error_ptr parse_invalid_reader_deref(read::source const &source)
  {
  }

  error_ptr parse_unresolved_namespace(read::source const &source)
  {
  }

  error_ptr parse_invalid_ratio(read::source const &source)
  {
  }

  error_ptr parse_invalid_escaped_string(read::source const &source)
  {
  }

  error_ptr parse_invalid_keyword(read::source const &source)
  {
  }

  error_ptr
  internal_parse_failure(native_persistent_string const &message, read::source const &source)
  {
  }

#pragma clang diagnostic pop
}
