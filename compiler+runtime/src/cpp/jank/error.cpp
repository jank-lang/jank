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
      case kind::lex_invalid_string_escape:
        return "Invalid string escape sequence.";
      case kind::lex_unexpected_character:
        return "Unexpected character.";
      case kind::internal_lex_failure:
        return "Internal lex failure.";

      case kind::parse_invalid_unicode:
        return "Invalid Unicode character.";
      case kind::parse_invalid_character:
        return "Invalid character.";
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
        return "Meta hint must be a keyword or map.";
      case kind::parse_invalid_meta_hint_target:
        return "Invalid meta hint target.";
      case kind::parse_unsupported_reader_macro:
        return "Unsupported reader macro.";
      case kind::parse_nested_shorthand_function:
        return "Nested #() forms are not allowed.";
      case kind::parse_invalid_shorthand_function:
        return "Invalid shorthand function.";
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
      case kind::internal_parse_failure:
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
      case kind::analyze_invalid_loop:
        return "Invalid loop.";
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
      case kind::analyze_invalid_conversion:
        return "Invalid conversion.";
      case kind::analyze_invalid_cpp_operator_call:
        return "Invalid C++ operator call.";
      case kind::analyze_invalid_cpp_constructor_call:
        return "Invalid C++ constructor call.";
      case kind::analyze_invalid_cpp_member_call:
        return "Invalid C++ member function call.";
      case kind::analyze_invalid_cpp_capture:
        return "Invalid C++ capture.";
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
      case kind::analyze_invalid_cpp_value:
        return "Invalid C++ value.";
      case kind::analyze_invalid_cpp_cast:
        return "Invalid C++ cast.";
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
      case kind::analyze_known_issue:
        return "Known issue.";
      case kind::internal_analyze_failure:
        return "Internal analysis failure.";

      case kind::internal_codegen_failure:
        return "Internal codegen failure.";

      case kind::aot_unresolved_main:
        return "Unresolved -main function.";
      case kind::aot_compilation_failure:
        return "Ahead-of-time compilation failure.";
      case kind::internal_aot_failure:
        return "Internal ahead-of-time compilation failure.";

      case kind::system_clang_executable_not_found:
        return "Unable to find a suitable Clang " JANK_CLANG_MAJOR_VERSION " binary.";
      case kind::system_failure:
        return "System failure.";

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
      case kind::internal_runtime_failure:
        return "Internal runtime failure.";

      case kind::internal_failure:
        return "Internal failure.";
    }
    return "Unknown error 😮!";
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
             std::unique_ptr<cpptrace::stacktrace> trace)
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
             std::unique_ptr<cpptrace::stacktrace> trace)
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
    }
    else
    {
      notes.emplace_back("Used here.", usage_source, note::kind::info);
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
    e->trace = std::make_unique<cpptrace::stacktrace>(cpptrace::generate_trace());
    return e;
  }

  void throw_internal_failure(jtl::immutable_string const &message)
  {
    throw internal_failure(message);
  }
}
