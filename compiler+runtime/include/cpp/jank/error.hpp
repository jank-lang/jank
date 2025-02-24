#pragma once

#include <jank/runtime/object.hpp>
#include <jank/read/source.hpp>
#include <jank/option.hpp>

namespace jank::error
{
  enum class kind : uint8_t
  {
    lex_unexpected_eof,
    lex_expecting_whitespace,
    lex_invalid_unicode,
    lex_incomplete_character,
    lex_invalid_number,
    lex_invalid_ratio,
    lex_invalid_symbol,
    lex_invalid_keyword,
    lex_unterminated_string,
    lex_invalid_string_escape,
    lex_unexpected_character,
    internal_lex_failure,

    parse_invalid_unicode,
    parse_invalid_character,
    parse_unexpected_closing_character,
    parse_unterminated_list,
    parse_unterminated_vector,
    parse_unterminated_map,
    parse_unterminated_set,
    parse_odd_entries_in_map,
    parse_invalid_quote,
    parse_invalid_meta_hint_value,
    parse_invalid_meta_hint_target,
    parse_unsupported_reader_macro,
    parse_nested_shorthand_function,
    parse_invalid_shorthand_function,
    parse_invalid_shorthand_function_parameter,
    parse_invalid_reader_var,
    parse_invalid_reader_comment,
    parse_invalid_reader_conditional,
    parse_invalid_reader_splice,
    parse_invalid_reader_gensym,
    parse_invalid_reader_symbolic_value,
    parse_invalid_syntax_quote,
    parse_invalid_syntax_unquote,
    parse_invalid_syntax_unquote_splice,
    parse_invalid_reader_deref,
    parse_invalid_ratio,
    parse_invalid_keyword,
    internal_parse_failure,

    analysis_invalid_case,
    analysis_invalid_def,
    analysis_invalid_fn,
    analysis_invalid_fn_parameters,
    analysis_invalid_recur_position,
    analysis_invalid_recur_from_try,
    analysis_invalid_recur_args,
    analysis_invalid_let,
    analysis_invalid_loop,
    analysis_invalid_if,
    analysis_invalid_quote,
    analysis_invalid_var_reference,
    analysis_invalid_throw,
    analysis_invalid_try,
    analysis_unresolved_var,
    analysis_unresolved_symbol,
    internal_analysis_failure,

    internal_codegen_failure,

    internal_runtime_failure,

    internal_failure,
  };

  constexpr char const *kind_str(kind const k)
  {
    switch(k)
    {
      case kind::lex_unexpected_eof:
        return "lex/unexpected-eof";
      case kind::lex_expecting_whitespace:
        return "lex/expecting-whitespace";
      case kind::lex_invalid_unicode:
        return "lex/invalid-unicode";
      case kind::lex_incomplete_character:
        return "lex/incomplete-character";
      case kind::lex_invalid_number:
        return "lex/invalid-number";
      case kind::lex_invalid_ratio:
        return "lex/invalid-ratio";
      case kind::lex_invalid_symbol:
        return "lex/invalid-symbol";
      case kind::lex_invalid_keyword:
        return "lex/invalid-keyword";
      case kind::lex_unterminated_string:
        return "lex/unterminated-string";
      case kind::lex_invalid_string_escape:
        return "lex/invalid-string-escape";
      case kind::lex_unexpected_character:
        return "lex/unexpected-character";
      case kind::internal_lex_failure:
        return "internal/lex-failure";
      case kind::parse_invalid_unicode:
        return "parse/invalid-unicode";
      case kind::parse_invalid_character:
        return "parse/invalid-character";
      case kind::parse_unexpected_closing_character:
        return "parse/unexpected-closing-character";
      case kind::parse_unterminated_list:
        return "parse/unterminated-list";
      case kind::parse_unterminated_vector:
        return "parse/unterminated-vector";
      case kind::parse_unterminated_map:
        return "parse/unterminated-map";
      case kind::parse_unterminated_set:
        return "parse/unterminated-set";
      case kind::parse_odd_entries_in_map:
        return "parse/odd-entries-in-map";
      case kind::parse_invalid_quote:
        return "parse/invalid-quote";
      case kind::parse_invalid_meta_hint_value:
        return "parse/invalid-meta-hint-value";
      case kind::parse_invalid_meta_hint_target:
        return "parse/invalid-meta-hint-target";
      case kind::parse_unsupported_reader_macro:
        return "parse/unsupported-reader-macro";
      case kind::parse_nested_shorthand_function:
        return "parse/nested-shorthand-function";
      case kind::parse_invalid_shorthand_function:
        return "parse/invalid-shorthand-function";
      case kind::parse_invalid_shorthand_function_parameter:
        return "parse_invalid_shorthand_function_parameter";
      case kind::parse_invalid_reader_var:
        return "parse/invalid-reader-var";
      case kind::parse_invalid_reader_comment:
        return "parse/invalid-reader-comment";
      case kind::parse_invalid_reader_conditional:
        return "parse/invalid-reader-conditional";
      case kind::parse_invalid_reader_splice:
        return "parse/invalid-reader-splice";
      case kind::parse_invalid_reader_gensym:
        return "parse/invalid-reader-gensym";
      case kind::parse_invalid_reader_symbolic_value:
        return "parse_invalid_reader_symbolic-value";
      case kind::parse_invalid_syntax_quote:
        return "parse/invalid-syntax-quote";
      case kind::parse_invalid_syntax_unquote:
        return "parse/invalid-syntax-unquote";
      case kind::parse_invalid_syntax_unquote_splice:
        return "parse/invalid-syntax-unquote-splice";
      case kind::parse_invalid_reader_deref:
        return "parse/invalid-reader-deref";
      case kind::parse_invalid_ratio:
        return "parse/invalid-ratio";
      case kind::parse_invalid_keyword:
        return "parse/invalid-keyword";
      case kind::internal_parse_failure:
        return "internal/parse-failure";
      case kind::analysis_invalid_case:
        return "analysis/invalid-case";
      case kind::analysis_invalid_def:
        return "analysis/invalid-def";
      case kind::analysis_invalid_fn:
        return "analysis/invalid-fn";
      case kind::analysis_invalid_fn_parameters:
        return "analysis/invalid-fn-parameters";
      case kind::analysis_invalid_recur_position:
        return "analysis/invalid-recur-position";
      case kind::analysis_invalid_recur_from_try:
        return "analysis/invalid-recur-from-try";
      case kind::analysis_invalid_recur_args:
        return "analysis/invalid-recur-args";
      case kind::analysis_invalid_let:
        return "analysis/invalid-let";
      case kind::analysis_invalid_loop:
        return "analysis/invalid-loop";
      case kind::analysis_invalid_if:
        return "analysis/invalid-if";
      case kind::analysis_invalid_quote:
        return "analysis/invalid-quote";
      case kind::analysis_invalid_var_reference:
        return "analysis/invalid-var-reference";
      case kind::analysis_invalid_throw:
        return "analysis/invalid-throw";
      case kind::analysis_invalid_try:
        return "analysis/invalid-try";
      case kind::analysis_unresolved_var:
        return "analysis/unresolved-var";
      case kind::analysis_unresolved_symbol:
        return "analysis/unresolved-symbol";
      case kind::internal_analysis_failure:
        return "internal/analysis-failure";
      case kind::internal_codegen_failure:
        return "internal/codegen-failure";
      case kind::internal_runtime_failure:
        return "internal/runtime-failure";
      case kind::internal_failure:
        return "internal/failure";
    }
    return "unknown";
  }

  struct note
  {
    enum class kind : uint8_t
    {
      info,
      warning,
      error
    };

    native_persistent_string message;
    read::source source;
    kind kind{ kind::error };
  };

  struct base : gc
  {
    base() = delete;
    base(base const &) = default;
    base(base &&) noexcept = default;
    base(kind k, read::source const &source);
    base(kind k, read::source const &source, native_vector<note> const &notes);
    base(kind k, native_persistent_string const &message, read::source const &source);
    base(kind k, read::source const &source, native_persistent_string const &note_message);
    base(kind k,
         native_persistent_string const &message,
         read::source const &source,
         native_persistent_string const &note_message);
    base(kind k, read::source const &source, note const &note);
    base(kind k,
         native_persistent_string const &message,
         read::source const &source,
         note const &note);
    base(kind k,
         native_persistent_string const &message,
         read::source const &source,
         native_vector<note> const &notes);

    native_bool operator==(base const &rhs) const;
    native_bool operator!=(base const &rhs) const;

    kind kind{};
    native_persistent_string message;
    read::source source;
    native_vector<note> notes;
    /* TODO: context */
    /* TODO: suggestions */
  };

  std::ostream &operator<<(std::ostream &os, base const &e);
}

namespace jank
{
  using error_ptr = runtime::native_box<error::base>;

  error_ptr make_error(error::kind const kind, native_persistent_string const &message);
  error_ptr make_error(error::kind const kind, read::source const &source);
  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source const &source);
  error_ptr
  make_error(error::kind const kind, read::source const &source, error::note const &error_note);
  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source const &source,
                       error::note const &error_note);
  error_ptr make_error(error::kind const kind,
                       read::source const &source,
                       native_persistent_string const &error_note_message);
  error_ptr make_error(error::kind const kind,
                       read::source const &source,
                       native_vector<error::note> const &notes);
  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source const &source,
                       native_persistent_string const &error_note_message);
  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start);
  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start,
                       read::source_position const &end);

  read::source meta_source(option<runtime::object_ptr> const &o);
}
