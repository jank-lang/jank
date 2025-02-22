#include <jank/error/parse.hpp>
#include <jank/util/string_builder.hpp>

namespace jank::error
{
  static constexpr char delim_char_for_token_kind(read::lex::token_kind const kind)
  {
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
      case read::lex::token_kind::single_quote:
      case read::lex::token_kind::meta_hint:
      case read::lex::token_kind::reader_macro:
      case read::lex::token_kind::reader_macro_comment:
      case read::lex::token_kind::reader_macro_conditional:
      case read::lex::token_kind::reader_macro_conditional_splice:
      case read::lex::token_kind::syntax_quote:
      case read::lex::token_kind::unquote:
      case read::lex::token_kind::unquote_splice:
      case read::lex::token_kind::deref:
      case read::lex::token_kind::comment:
      case read::lex::token_kind::nil:
      case read::lex::token_kind::boolean:
      case read::lex::token_kind::character:
      case read::lex::token_kind::symbol:
      case read::lex::token_kind::keyword:
      case read::lex::token_kind::integer:
      case read::lex::token_kind::real:
      case read::lex::token_kind::ratio:
      case read::lex::token_kind::string:
      case read::lex::token_kind::escaped_string:
      case read::lex::token_kind::eof:
        return '?';
    }
  }

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
    /* TODO: Add a note to show last open token. */
    util::string_builder sb;
    return make_error(
      kind::parse_unexpected_closing_character,
      sb("Unexpected closing character '")(delim_char_for_token_kind(token.kind))("'").release(),
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
    return make_error(kind::parse_invalid_meta_hint_value, source, "Meta hint is here");
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
                      native_vector<note>{
                        { "Inner #() starts here", source },
                        parent_fn_source
    });
  }

  error_ptr
  parse_invalid_shorthand_function(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_shorthand_function, source, note);
  }

  error_ptr parse_invalid_shorthand_function_parameter(read::source const &source)
  {
    return make_error(kind::parse_invalid_shorthand_function,
                      source,
                      "Arg literal must be %, %&, or %n where n >= 1");
  }

  error_ptr
  parse_invalid_reader_var(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_reader_var, "Invalid var reader macro", source, note);
  }

  error_ptr
  parse_invalid_reader_comment(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_reader_comment, source, note);
  }

  error_ptr
  parse_invalid_reader_conditional(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_reader_conditional, source, note);
  }

  error_ptr
  parse_invalid_reader_splice(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_reader_splice, source, note);
  }

  error_ptr parse_invalid_reader_gensym(read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_gensym, source);
  }

  error_ptr parse_invalid_reader_symbolic_value(native_persistent_string const &message,
                                                read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_symbolic_value, message, source);
  }

  error_ptr
  parse_invalid_syntax_quote(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_syntax_quote, source, note);
  }

  error_ptr parse_invalid_syntax_unquote(read::source const &source)
  {
    return make_error(kind::parse_invalid_syntax_unquote, source);
  }

  error_ptr parse_invalid_syntax_unquote_splice(read::source const &source)
  {
    return make_error(kind::parse_invalid_syntax_unquote_splice, source);
  }

  error_ptr parse_invalid_reader_deref(read::source const &source)
  {
    return make_error(kind::parse_invalid_reader_deref, source);
  }

  error_ptr parse_invalid_ratio(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_ratio, source, note);
  }

  error_ptr parse_invalid_keyword(read::source const &source, native_persistent_string const &note)
  {
    return make_error(kind::parse_invalid_keyword, source, note);
  }

  error_ptr
  internal_parse_failure(native_persistent_string const &message, read::source const &source)
  {
    return make_error(kind::internal_parse_failure, message, source);
  }

  error_ptr internal_parse_failure(native_persistent_string const &message)
  {
    return make_error(kind::internal_parse_failure, message, read::source::unknown);
  }
}
