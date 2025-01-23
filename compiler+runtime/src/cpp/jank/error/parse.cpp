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

  error_ptr parse_invalid_unicode(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_unicode, source, note);
  }

  error_ptr parse_invalid_character(read::lex::token const &token)
  {
    util::string_builder sb;
    return make_error(
      kind::parse_invalid_character,
      sb("Invalid character '")(boost::get<native_persistent_string_view>(token.data))("'")
        .release(),
      { token.start, token.end },
      "Found here");
  }

  error_ptr parse_unexpected_closing_character(read::lex::token const &token)
  {
    /* TODO: Point to last open char? */
    util::string_builder sb;
    return make_error(
      kind::parse_unexpected_closing_character,
      sb("Unexpected closing character '")(char_for_token_kind(token.kind))("'").release(),
      token.start,
      "This is unexpected, since it has no matching open character");
  }

  error_ptr parse_unterminated_list(read::source const &source)
  {
    return make_error(kind::parse_unterminated_list,
                      source,
                      note{ "List started here", source.start });
  }

  error_ptr parse_unterminated_vector(read::source const &source)
  {
    return make_error(kind::parse_unterminated_vector,
                      source,
                      note{ "Vector started here", source.start });
  }

  error_ptr parse_unterminated_map(read::source const &source)
  {
    return make_error(kind::parse_unterminated_map,
                      source,
                      note{ "Map started here", source.start });
  }

  error_ptr parse_unterminated_set(read::source const &source)
  {
    return make_error(kind::parse_unterminated_set,
                      source,
                      note{ "Set started here", source.start });
  }

  error_ptr
  parse_odd_entries_in_map(read::source const &map_source, read::source const &last_key_source)
  {
    return make_error(kind::parse_odd_entries_in_map,
                      map_source,
                      note{ "No value for this key", last_key_source });
  }

  error_ptr parse_invalid_quote(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_quote, source, note);
  }

  error_ptr parse_invalid_meta_hint_value(read::source const &source)
  {
    return make_error(kind::parse_invalid_meta_hint_value, source, "Excepting value here");
  }

  error_ptr
  parse_invalid_meta_hint_target(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_meta_hint_target, source, note);
  }

  error_ptr parse_unsupported_reader_macro(read::source const &source)
  {
    return make_error(kind::parse_unsupported_reader_macro, source);
  }

  error_ptr
  parse_nested_shorthand_function(read::source const &source, note const &parent_fn_source)
  {
    return make_error(kind::parse_nested_shorthand_function,
                      source,
                      "Inner #() starts here",
                      parent_fn_source);
  }

  error_ptr parse_invalid_shorthand_function(native_persistent_string const &message,
                                             read::source const &source)
  {
    return make_error(kind::parse_invalid_shorthand_function, source, message);
  }

  error_ptr parse_invalid_shorthand_function_parameter(read::source const &source)
  {
    return make_error(kind::parse_invalid_shorthand_function,
                      source,
                      "Arg literal must be %, %&, or %n where n is an integer");
  }

  error_ptr
  parse_invalid_reader_var(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_var, "Invalid var reader macro", source, message);
  }

  error_ptr
  parse_invalid_reader_comment(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_comment, source, message);
  }

  error_ptr parse_invalid_reader_conditional(native_persistent_string const &message,
                                             read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_conditional, source, message);
  }

  error_ptr
  parse_invalid_reader_splice(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_splice, source, message);
  }

  error_ptr parse_invalid_reader_gensym(read::source const &source)
  {
  }

  error_ptr
  parse_invalid_syntax_quote(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_invalid_syntax_quote, source, message);
  }

  error_ptr parse_invalid_syntax_unquote(read::source const &source)
  {
    return make_error(kind::parse_invalid_syntax_unquote, source);
  }

  error_ptr parse_invalid_reader_deref(read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_deref, source);
  }

  error_ptr
  parse_unresolved_namespace(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_unresolved_namespace, message, source, "Referenced here");
  }

  error_ptr parse_invalid_ratio(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_invalid_ratio, source, message);
  }

  error_ptr
  parse_invalid_keyword(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::parse_invalid_keyword, source, message);
  }

  error_ptr
  internal_parse_failure(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::internal_parse_failure, source, message);
  }

#pragma clang diagnostic pop
}
