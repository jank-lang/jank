#include <cpptrace/cpptrace.hpp>

#include <jank/error.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/keyword.hpp>

namespace jank::error
{
  static constexpr auto default_note_message{ "Found here." };

  static constexpr jtl::immutable_string_view kind_to_message(kind const k)
  {
    switch(k)
    {
      case kind::lex_unexpected_eof:
        return "Unexpected end of file.";
      case kind::lex_expecting_whitespace:
        return "Expecting whitespace after the last token.";
      case kind::lex_invalid_unicode:
        return "Invalid Unicode character.";
      case kind::lex_incomplete_character:
        return "Incomplete character.";
      case kind::lex_invalid_number:
        return "Invalid number.";
      case kind::lex_invalid_ratio:
        return "Invalid ratio.";
      case kind::lex_invalid_symbol:
        return "Invalid symbol.";
      case kind::lex_invalid_keyword:
        return "Invalid keyword.";
      case kind::lex_unterminated_string:
        return "Unterminated string.";
      case kind::lex_unexpected_character:
        return "Unexpected character.";
      case kind::lex_internal_failure:
        return "Internal lex failure.";

      case kind::parse_invalid_unicode:
        return "Invalid Unicode character.";
      case kind::parse_invalid_character:
        return "Invalid character.";
      case kind::parse_invalid_string_escape:
        return "Invalid string escape sequence.";
      case kind::parse_unexpected_closing_character:
        return "Unexpected closing character.";
      case kind::parse_unterminated_list:
        return "Unterminated list.";
      case kind::parse_unterminated_vector:
        return "Unterminated vector.";
      case kind::parse_unterminated_map:
        return "Unterminated map.";
      case kind::parse_unterminated_set:
        return "Unterminated set.";
      case kind::parse_odd_entries_in_map:
        return "Odd number of entries in map.";
      case kind::parse_duplicate_keys_in_map:
        return "Duplicate keys in map literals are not allowed.";
      case kind::parse_duplicate_items_in_set:
        return "Duplicate items in set literals are not allowed.";
      case kind::parse_invalid_quote:
        return "Invalid quote.";
      case kind::parse_invalid_meta_hint_value:
        return "Meta hint must be a keyword, symbol, list, or map.";
      case kind::parse_invalid_meta_hint_target:
        return "Invalid meta hint target.";
      case kind::parse_unsupported_reader_macro:
        return "Unsupported reader macro.";
      case kind::parse_nested_shorthand_function:
        return "Nested #() forms are not allowed.";
      case kind::parse_invalid_shorthand_function_parameter:
        return "Invalid shorthand function parameter.";
      case kind::parse_invalid_reader_var:
        return "Invalid reader var reference.";
      case kind::parse_invalid_reader_comment:
        return "Invalid reader comment.";
      case kind::parse_invalid_reader_conditional:
        return "Invalid reader conditional.";
      case kind::parse_invalid_reader_splice:
        return "Invalid reader splice.";
      case kind::parse_invalid_reader_gensym:
        return "gensym literal is not within a syntax quote.";
      case kind::parse_invalid_reader_symbolic_value:
        return "Invalid reader symbolic value.";
      case kind::parse_invalid_reader_tag_value:
        return "Invalid reader tag value.";
      case kind::parse_invalid_regex:
        return "Invalid regex.";
      case kind::parse_invalid_uuid:
        return "Invalid UUID.";
      case kind::parse_invalid_inst:
        return "Unsupported date/time syntax.";
      case kind::parse_invalid_syntax_quote:
        return "Invalid syntax quote.";
      case kind::parse_invalid_syntax_unquote:
        return "Invalid syntax unquote.";
      case kind::parse_invalid_syntax_unquote_splice:
        return "Unquote splice is not within a sequence.";
      case kind::parse_invalid_reader_deref:
        return "Invalid reader deref.";
      case kind::parse_invalid_ratio:
        return "Invalid ratio.";
      case kind::parse_invalid_keyword:
        return "Invalid keyword.";
      case kind::parse_invalid_data_reader:
        return "Invalid data reader.";
      case kind::parse_internal_failure:
        return "Internal parse failure.";

      case kind::analyze_invalid_case:
        return "Invalid case.";
      case kind::analyze_invalid_def:
        return "Invalid def.";
      case kind::analyze_invalid_fn:
        return "Invalid fn.";
      case kind::analyze_invalid_fn_parameters:
        return "Invalid fn parameters.";
      case kind::analyze_invalid_recur_position:
        return "recur must be used from tail position.";
      case kind::analyze_invalid_recur_from_try:
        return "recur may not be used within a 'try'.";
      case kind::analyze_invalid_recur_args:
        return "The argument arity passed to 'recur' doesn't match the function's arity.";
      case kind::analyze_invalid_let:
        return "Invalid let.";
      case kind::analyze_invalid_letfn:
        return "Invalid letfn.";
      case kind::analyze_invalid_if:
        return "Invalid if.";
      case kind::analyze_invalid_quote:
        return "Invalid quote.";
      case kind::analyze_invalid_var_reference:
        return "Invalid var reference.";
      case kind::analyze_invalid_throw:
        return "Invalid throw.";
      case kind::analyze_invalid_try:
        return "Invalid try.";
      case kind::analyze_unresolved_var:
        return "Unresolved var.";
      case kind::analyze_unresolved_symbol:
        return "Unresolved symbol.";
      case kind::analyze_macro_expansion_exception:
        return "Macro expansion exception.";
      case kind::analyze_invalid_cpp_operator_call:
        return "Invalid C++ operator call.";
      case kind::analyze_invalid_cpp_constructor_call:
        return "Invalid C++ constructor call.";
      case kind::analyze_invalid_cpp_member_call:
        return "Invalid C++ member function call.";
      case kind::analyze_invalid_cpp_capture:
        return "Invalid C++ capture.";
      case kind::analyze_invalid_cpp_position:
        return "Unable to use this form as a first-class value. It needs to be called directly.";
      case kind::analyze_mismatched_if_types:
        return "Mismatched if types.";
      case kind::analyze_invalid_cpp_function_call:
        return "Invalid C++ function call.";
      case kind::analyze_invalid_cpp_call:
        return "Invalid C++ call.";
      case kind::analyze_invalid_cpp_conversion:
        return "Invalid C++ type returned.";
      case kind::analyze_invalid_cpp_symbol:
        return "Invalid C++ symbol.";
      case kind::analyze_unresolved_cpp_symbol:
        return "Unresolvable C++ symbol.";
      case kind::analyze_invalid_cpp_raw:
        return "Invalid C++ raw.";
      case kind::analyze_invalid_cpp_type:
        return "Invalid C++ type.";
      case kind::analyze_invalid_cpp_type_position:
        return "Invalid position for a C++ type.";
      case kind::analyze_invalid_cpp_dsl:
        return "Invalid C++ type form.";
      case kind::analyze_invalid_cpp_value:
        return "Invalid C++ value.";
      case kind::analyze_invalid_cpp_cast:
        return "Invalid C++ cast.";
      case kind::analyze_invalid_cpp_unsafe_cast:
        return "Invalid C++ unsafe-cast.";
      case kind::analyze_invalid_cpp_box:
        return "Invalid C++ box.";
      case kind::analyze_invalid_cpp_unbox:
        return "Invalid C++ unbox.";
      case kind::analyze_invalid_cpp_new:
        return "Invalid C++ new.";
      case kind::analyze_invalid_cpp_delete:
        return "Invalid C++ delete.";
      case kind::analyze_invalid_cpp_member_access:
        return "Invalid C++ member access.";
      case kind::analyze_internal_failure:
        return "Internal analysis failure.";

      case kind::codegen_internal_failure:
        return "Internal codegen failure.";

      case kind::aot_unresolved_main:
        return "Unresolved -main function.";
      case kind::aot_internal_failure:
        return "Internal ahead-of-time compilation failure.";

      case kind::runtime_module_not_found:
        return "Module not found.";
      case kind::runtime_module_binary_without_source:
        return "Module binary found, but no corresponding source was found.";
      case kind::runtime_unable_to_open_file:
        return "Unable to open file.";
      case kind::runtime_invalid_cpp_eval:
        return "Unable to compile the provided C++ source.";
      case kind::runtime_unable_to_load_module:
        return "Unable to load module.";
      case kind::runtime_invalid_unbox:
        return "Invalid unbox type.";
      case kind::runtime_non_metadatable_value:
        return "Non metadatable value.";
      case kind::runtime_invalid_referred_global_symbol:
        return "Invalid referred C++ global symbol.";
      case kind::runtime_invalid_referred_global_rename:
        return "Invalid referred C++ global rename.";
      case kind::runtime_unsupported_behavior:
        return "Unsupported behavior.";
      case kind::runtime_static_feature_disabled:
        return "This feature is disabled in a static runtime.";
      case kind::runtime_uncaught_exception:
        return "Uncaught exception.";
      case kind::runtime_internal_failure:
        return "Internal runtime failure.";

      case kind::system_clang_executable_not_found:
        return "Unable to find a suitable Clang " JANK_CLANG_MAJOR_VERSION " binary.";
      case kind::system_failure:
        return "System failure.";

      case kind::internal_failure:
        return "Internal failure.";
    }
    return "Unknown error 😮!";
  }

  char const *kind_str(kind const k)
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
      case kind::lex_unexpected_character:
        return "lex/unexpected-character";
      case kind::lex_internal_failure:
        return "lex/internal-failure";

      case kind::parse_invalid_unicode:
        return "parse/invalid-unicode";
      case kind::parse_invalid_character:
        return "parse/invalid-character";
      case kind::parse_invalid_string_escape:
        return "parse/invalid-string-escape";
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
        return "parse/invalid-reader-symbolic-value";
      case kind::parse_invalid_reader_tag_value:
        return "parse/invalid-reader-tag-value";
      case kind::parse_invalid_regex:
        return "parse/invalid-regex";
      case kind::parse_invalid_uuid:
        return "parse/invalid-uuid";
      case kind::parse_invalid_inst:
        return "parse/invalid-inst";
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
      case kind::parse_invalid_data_reader:
        return "parse/invalid-data-reader";
      case kind::parse_internal_failure:
        return "parse/internal-failure";

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

      case kind::analyze_invalid_cpp_operator_call:
        return "analyze/invalid-cpp-operator-call";
      case kind::analyze_invalid_cpp_constructor_call:
        return "analyze/invalid-cpp-constructor-call";
      case kind::analyze_invalid_cpp_member_call:
        return "analyze/invalid-cpp-member-call";
      case kind::analyze_invalid_cpp_function_call:
        return "analyze/invalid-cpp-function-call";
      case kind::analyze_invalid_cpp_call:
        return "analyze/invalid-cpp-call";
      case kind::analyze_invalid_cpp_conversion:
        return "analyze/invalid-cpp-conversion";
      case kind::analyze_invalid_cpp_symbol:
        return "analyze/invalid-cpp-symbol";
      case kind::analyze_unresolved_cpp_symbol:
        return "analyze/unresolved-cpp-symbol";
      case kind::analyze_invalid_cpp_raw:
        return "analyze/invalid-cpp-raw";
      case kind::analyze_invalid_cpp_type:
        return "analyze/invalid-cpp-type";
      case kind::analyze_invalid_cpp_type_position:
        return "analyze/invalid-cpp-type-position";
      case kind::analyze_invalid_cpp_dsl:
        return "analyze/invalid-cpp-dsl";
      case kind::analyze_invalid_cpp_value:
        return "analyze/invalid-cpp-value";
      case kind::analyze_invalid_cpp_cast:
        return "analyze/invalid-cpp-cast";
      case kind::analyze_invalid_cpp_unsafe_cast:
        return "analyze/invalid-cpp-unsafe-cast";
      case kind::analyze_invalid_cpp_box:
        return "analyze/invalid-cpp-box";
      case kind::analyze_invalid_cpp_unbox:
        return "analyze/invalid-cpp-unbox";
      case kind::analyze_invalid_cpp_new:
        return "analyze/invalid-cpp-new";
      case kind::analyze_invalid_cpp_delete:
        return "analyze/invalid-cpp-delete";
      case kind::analyze_invalid_cpp_member_access:
        return "analyze/invalid-cpp-member-access";
      case kind::analyze_invalid_cpp_capture:
        return "analyze/invalid-cpp-capture";
      case kind::analyze_invalid_cpp_position:
        return "analyze/invalid-cpp-position";
      case kind::analyze_mismatched_if_types:
        return "analyze/mismatched-if-types";
      case kind::analyze_internal_failure:
        return "analyze/internal-failure";

      case kind::codegen_internal_failure:
        return "codegen/internal-failure";

      case kind::aot_unresolved_main:
        return "aot/unresolved-main";
      case kind::aot_internal_failure:
        return "aot/internal-failure";

      case kind::runtime_module_not_found:
        return "runtime/module-not-found";
      case kind::runtime_module_binary_without_source:
        return "runtime/module-binary-without-source";
      case kind::runtime_unable_to_open_file:
        return "runtime/unable-to-open-file";
      case kind::runtime_invalid_cpp_eval:
        return "runtime/invalid-cpp-eval";
      case kind::runtime_unable_to_load_module:
        return "runtime/unable-to-load-module";
      case kind::runtime_invalid_unbox:
        return "runtime/invalid-unbox";
      case kind::runtime_non_metadatable_value:
        return "runtime/non-metadatable-value";
      case kind::runtime_invalid_referred_global_symbol:
        return "runtime/invalid-referred-global-symbol";
      case kind::runtime_invalid_referred_global_rename:
        return "runtime/invalid-referred-global-rename";
      case kind::runtime_unsupported_behavior:
        return "runtime/unsupported-behavior";
      case kind::runtime_static_feature_disabled:
        return "runtime/static-feature-disabled";
      case kind::runtime_uncaught_exception:
        return "runtime/uncaught-exception";
      case kind::runtime_internal_failure:
        return "runtime/internal-failure";

      case kind::system_clang_executable_not_found:
        return "system/clang-executable-not-found";
      case kind::system_failure:
        return "system/failure";

      case kind::internal_failure:
        return "internal/failure";
    }
    return "unknown";
  }

  bool is_internal_failure_kind(kind const k)
  {
    return k == kind::lex_internal_failure || k == kind::parse_internal_failure
      || k == kind::analyze_internal_failure || k == kind::codegen_internal_failure
      || k == kind::aot_internal_failure || k == kind::runtime_internal_failure
      || k == kind::internal_failure;
  }

  jtl::immutable_string note::to_string() const
  {
    jtl::string_builder sb;
    return sb("note(\"")(message)("\", ")(source.to_string())(", ")(note::kind_str(kind))(")")
      .release();
  }

  static void add_expansion_note(base &e, runtime::object_ref const expansion)
  {
    auto source{ runtime::object_source(expansion) };
    if(source == read::source::unknown())
    {
      return;
    }

    /* We just want to point at the start of the expansion, not underline the
     * whole thing. It may be huge! */
    source.end = source.start;
    e.notes.emplace_back("Expanded from this macro.", source, note::kind::info);
  }

  base::base(enum kind const k, read::source const &source)
    : kind{
      k
  }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ { default_note_message, source } }
  {
  }

  base::base(enum kind const k, read::source const &source, native_vector<note> const &notes)
    : kind{ k }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ notes }
  {
  }

  base::base(enum kind const k, read::source const &source, runtime::object_ref const expansion)
    : kind{
      k
  }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ { default_note_message, source } }
  {
    add_expansion_note(*this, expansion);
  }

  base::base(enum kind const k, jtl::immutable_string const &message, read::source const &source)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { default_note_message, source } }
  {
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             runtime::object_ref const expansion)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { default_note_message, source } }
  {
    add_expansion_note(*this, expansion);
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             runtime::object_ref const expansion,
             std::unique_ptr<cpptrace::raw_trace> trace)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { default_note_message, source } },
    trace{ std::move(trace) }
  {
    add_expansion_note(*this, expansion);
  }

  base::base(enum kind const k,
             read::source const &source,
             jtl::immutable_string const &note_message)
    : kind{
      k
  }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ { note_message, source } }
  {
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             jtl::immutable_string const &note_message)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { note_message, source } }
  {
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             jtl::immutable_string const &note_message,
             runtime::object_ref const expansion)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { note_message, source } }
  {
    add_expansion_note(*this, expansion);
  }

  base::base(enum kind const k, read::source const &source, note const &note)
    : kind{ k }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ note }
  {
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             note const &note)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{ note }
  {
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             note const &note,
             runtime::object_ref const expansion)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{ note }
  {
    add_expansion_note(*this, expansion);
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             native_vector<note> const &notes)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{ notes }
  {
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             runtime::object_ref const expansion,
             jtl::ref<base> const cause)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { default_note_message, source } },
    cause{ cause }
  {
    add_expansion_note(*this, expansion);
  }

  base::base(enum kind const k,
             jtl::immutable_string const &message,
             read::source const &source,
             runtime::object_ref const expansion,
             jtl::ref<base> const cause,
             std::unique_ptr<cpptrace::raw_trace> trace)
    : kind{
      k
  }
    , message{ message }
    , source{ source }
    , notes{ { default_note_message, source } },
    cause{ cause }, trace{ std::move(trace) }
  {
    add_expansion_note(*this, expansion);
  }

  bool base::operator==(base const &rhs) const
  {
    return !(*this != rhs);
  }

  bool base::operator!=(base const &rhs) const
  {
    return kind != rhs.kind || source != rhs.source || message != rhs.message;
  }

  /* Sort notes by file, line, and column. This makes it easier to add them
   * sequentially and know they're going top-to-bottom and left-to-right.
   * Without sorting them, you cannot know that. */
  void base::sort_notes()
  {
    std::ranges::stable_sort(notes, [](note const &lhs, note const &rhs) -> bool {
      return lhs.source.start.col < rhs.source.start.col;
    });
    std::ranges::stable_sort(notes, [](note const &lhs, note const &rhs) -> bool {
      return lhs.source.start.line < rhs.source.start.line;
    });
    std::ranges::stable_sort(notes, [](note const &lhs, note const &rhs) -> bool {
      return lhs.source.file < rhs.source.file;
    });
  }

  /* When we create errors, we may want to point at where the error happened, which we
   * call the usage here. In some cases, the usage is identical to what we already
   * identified as the source of the error. For those cases, adding the usage does nothing.
   * For other cases, we'll add an additional note. There's also a final case where
   * the current error has an unknown source, since we didn't have a good source to
   * begin with. In that case, we update the existing note rather than adding a new one. */
  jtl::ref<base> base::add_usage(read::source const &usage_source)
  {
    if(usage_source == read::source::unknown() || usage_source.overlaps(source))
    {
      return this;
    }
    else if(source == read::source::unknown())
    {
      source = usage_source;
      notes[0].source = usage_source;
      return this;
    }

    for(auto const &note : notes)
    {
      if(usage_source.overlaps(note.source))
      {
        return this;
      }
    }

    notes.emplace_back("Used here.", usage_source, note::kind::info);
    return this;
  }

  /* This is similar to `add_usage`, but it only adds a source if there is none. This is
   * just a way to ensure we have _something_ to show to the user. */
  jtl::ref<base> base::add_fallback_usage(read::source const &usage_source)
  {
    if(usage_source == read::source::unknown() || usage_source.overlaps(source))
    {
      return this;
    }
    else if(source == read::source::unknown() && notes[0].source == read::source::unknown()
            && notes.size() == 1)
    {
      source = usage_source;
      notes[0].source = usage_source;
    }
    return this;
  }

  std::ostream &operator<<(std::ostream &os, base const &e)
  {
    return os << "error(" << kind_str(e.kind) << " - " << e.source << ", \"" << e.message << "\")";
  }

  error_ref internal_failure(jtl::immutable_string const &message)
  {
    auto const e{ make_error(kind::internal_failure, message, read::source::unknown()) };
    e->trace = std::make_unique<cpptrace::raw_trace>(cpptrace::generate_raw_trace());
    return e;
  }

  void throw_internal_failure(jtl::immutable_string const &message)
  {
    throw internal_failure(message);
  }

  void throw_result_failure(jtl::immutable_string const &message)
  {
    auto const e{ make_error(kind::runtime_uncaught_exception, message, read::source::unknown()) };
    e->trace = std::make_unique<cpptrace::raw_trace>(cpptrace::generate_raw_trace());
    throw e;
  }
}
