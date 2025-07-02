#pragma once

#include <jtl/ptr.hpp>

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/read/source.hpp>

namespace cpptrace
{
  struct stacktrace;
}

namespace jank::error
{
  enum class kind : u8
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
    parse_duplicate_keys_in_map,
    parse_duplicate_items_in_set,
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

    analyze_invalid_case,
    analyze_invalid_def,
    analyze_invalid_fn,
    analyze_invalid_fn_parameters,
    analyze_invalid_recur_position,
    analyze_invalid_recur_from_try,
    analyze_invalid_recur_args,
    analyze_invalid_let,
    analyze_invalid_letfn,
    analyze_invalid_loop,
    analyze_invalid_if,
    analyze_invalid_quote,
    analyze_invalid_var_reference,
    analyze_invalid_throw,
    analyze_invalid_try,
    analyze_unresolved_var,
    analyze_unresolved_symbol,
    analyze_macro_expansion_exception,
    aot_compilation_failure,
    aot_clang_executable_not_found,
    aot_module_not_found,
    internal_analyze_failure,

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
      case kind::parse_duplicate_keys_in_map:
        return "parse/duplicate-keys-in-map";
      case kind::parse_duplicate_items_in_set:
        return "parse/duplicate-items-in-set";
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
      case kind::analyze_invalid_case:
        return "analyze/invalid-case";
      case kind::analyze_invalid_def:
        return "analyze/invalid-def";
      case kind::analyze_invalid_fn:
        return "analyze/invalid-fn";
      case kind::analyze_invalid_fn_parameters:
        return "analyze/invalid-fn-parameters";
      case kind::analyze_invalid_recur_position:
        return "analyze/invalid-recur-position";
      case kind::analyze_invalid_recur_from_try:
        return "analyze/invalid-recur-from-try";
      case kind::analyze_invalid_recur_args:
        return "analyze/invalid-recur-args";
      case kind::analyze_invalid_let:
        return "analyze/invalid-let";
      case kind::analyze_invalid_letfn:
        return "analyze/invalid-letfn";
      case kind::analyze_invalid_loop:
        return "analyze/invalid-loop";
      case kind::analyze_invalid_if:
        return "analyze/invalid-if";
      case kind::analyze_invalid_quote:
        return "analyze/invalid-quote";
      case kind::analyze_invalid_var_reference:
        return "analyze/invalid-var-reference";
      case kind::analyze_invalid_throw:
        return "analyze/invalid-throw";
      case kind::analyze_invalid_try:
        return "analyze/invalid-try";
      case kind::analyze_unresolved_var:
        return "analyze/unresolved-var";
      case kind::analyze_unresolved_symbol:
        return "analyze/unresolved-symbol";
      case kind::analyze_macro_expansion_exception:
        return "analyze/macro-expansion-exception";
      case kind::aot_compilation_failure:
        return "aot/compilation_failure";
      case kind::aot_clang_executable_not_found:
        return "aot/clang_executable_not_found";
      case kind::aot_module_not_found:
        return "aot/module_not_found";

      case kind::internal_analyze_failure:
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
    enum class kind : u8
    {
      info,
      warning,
      error
    };

    static constexpr char const *kind_str(kind const k)
    {
      switch(k)
      {
        case kind::info:
          return "info";
        case kind::warning:
          return "warning";
        case kind::error:
          return "error";
      }
      return "unknown";
    }

    jtl::immutable_string to_string() const;

    jtl::immutable_string message;
    read::source source;
    kind kind{ kind::error };
  };

  /* We need gc_cleanup to run the dtor for the unique_ptr<stacktrace>. This
   * is because cpptrace doesn't use our GC allocator. */
  struct base : gc_cleanup
  {
    base() = delete;
    base(base const &) = delete;
    base(base &&) noexcept = default;
    base(kind k, read::source const &source);
    base(kind k, read::source const &source, native_vector<note> const &notes);
    base(kind k, jtl::immutable_string const &message, read::source const &source);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref expansion);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref expansion,
         std::unique_ptr<cpptrace::stacktrace> trace);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         jtl::immutable_string const &note_message,
         runtime::object_ref expansion);
    base(kind k, read::source const &source, jtl::immutable_string const &note_message);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         jtl::immutable_string const &note_message);
    base(kind k, read::source const &source, note const &note);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         note const &note);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         note const &note,
         runtime::object_ref expansion);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         native_vector<note> const &notes);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref expansion,
         jtl::ref<base> cause);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref expansion,
         jtl::ref<base> cause,
         std::unique_ptr<cpptrace::stacktrace> trace);

    bool operator==(base const &rhs) const;
    bool operator!=(base const &rhs) const;

    void sort_notes();
    jtl::ref<base> add_usage(read::source const &usage_source);

    kind kind{};
    jtl::immutable_string message;
    read::source source;
    native_vector<note> notes;
    jtl::ptr<base> cause;
    std::unique_ptr<cpptrace::stacktrace> trace;
    /* TODO: context */
    /* TODO: suggestions */
  };

  std::ostream &operator<<(std::ostream &os, base const &e);
}

namespace jank
{
  using error_ref = jtl::ref<error::base>;

  template <typename... Args>
  error_ref make_error(Args &&...args)
  {
    return jtl::make_ref<error::base>(jtl::forward<Args>(args)...);
  }

  namespace error
  {
    error_ref internal_failure(jtl::immutable_string const &message);
  }
}
