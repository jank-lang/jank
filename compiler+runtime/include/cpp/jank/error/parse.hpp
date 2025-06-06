#pragma once

#include <jank/error.hpp>
#include <jank/read/lex.hpp>

namespace jank::error
{
  error_ref parse_invalid_unicode(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_invalid_character(read::lex::token const &token);
  error_ref parse_unexpected_closing_character(read::lex::token const &token);
  error_ref parse_unterminated_list(read::source const &source);
  error_ref parse_unterminated_vector(read::source const &source);
  error_ref parse_unterminated_map(read::source const &source);
  error_ref parse_unterminated_set(read::source const &source);
  error_ref
  parse_odd_entries_in_map(read::source const &map_source, read::source const &last_key_source);
  error_ref parse_duplicate_keys_in_map(read::source const &duplicate_key_source,
                                        note const &original_key_source);
  error_ref parse_duplicate_items_in_set(read::source const &duplicate_item_source,
                                         note const &original_item_source);
  error_ref parse_invalid_quote(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_invalid_meta_hint_value(read::source const &source);
  error_ref
  parse_invalid_meta_hint_target(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_unsupported_reader_macro(read::source const &source);
  error_ref
  parse_nested_shorthand_function(read::source const &source, note const &parent_fn_source);
  error_ref
  parse_invalid_shorthand_function(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_invalid_shorthand_function_parameter(read::source const &source);
  error_ref parse_invalid_reader_var(read::source const &source, jtl::immutable_string const &note);
  error_ref
  parse_invalid_reader_comment(read::source const &source, jtl::immutable_string const &note);
  error_ref
  parse_invalid_reader_conditional(read::source const &source, jtl::immutable_string const &note);
  error_ref
  parse_invalid_reader_splice(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_invalid_reader_gensym(read::source const &source);
  error_ref parse_invalid_reader_symbolic_value(jtl::immutable_string const &message,
                                                read::source const &source);
  error_ref
  parse_invalid_syntax_quote(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_invalid_syntax_unquote(read::source const &source);
  error_ref parse_invalid_syntax_unquote_splice(read::source const &source);
  error_ref parse_invalid_reader_deref(read::source const &source);
  error_ref parse_invalid_ratio(read::source const &source, jtl::immutable_string const &note);
  error_ref parse_invalid_keyword(jtl::immutable_string const &message, read::source const &source);
  error_ref
  internal_parse_failure(jtl::immutable_string const &message, read::source const &source);
  error_ref internal_parse_failure(jtl::immutable_string const &message);
}
