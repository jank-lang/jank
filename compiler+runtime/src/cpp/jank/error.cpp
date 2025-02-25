#include <jank/error.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/obj/keyword.hpp>

namespace jank::error
{
  static constexpr native_persistent_string_view kind_to_message(kind const k)
  {
    switch(k)
    {
      case kind::lex_unexpected_eof:
        return "Unexpected end of file";
      case kind::lex_expecting_whitespace:
        return "Expecting whitespace after the last token";
      case kind::lex_invalid_unicode:
        return "Invalid Unicode character";
      case kind::lex_incomplete_character:
        return "Incomplete character";
      case kind::lex_invalid_number:
        return "Invalid number";
      case kind::lex_invalid_ratio:
        return "Invalid ratio";
      case kind::lex_invalid_symbol:
        return "Invalid symbol";
      case kind::lex_invalid_keyword:
        return "Invalid keyword";
      case kind::lex_unterminated_string:
        return "Unterminated string";
      case kind::lex_invalid_string_escape:
        return "Invalid string escape sequence";
      case kind::lex_unexpected_character:
        return "Unexpected character";
      case kind::internal_lex_failure:
        return "Internal lex failure";

      case kind::parse_invalid_unicode:
        return "Invalid Unicode character";
      case kind::parse_invalid_character:
        return "Invalid character";
      case kind::parse_unexpected_closing_character:
        return "Unexpected closing character";
      case kind::parse_unterminated_list:
        return "Unterminated list";
      case kind::parse_unterminated_vector:
        return "Unterminated vector";
      case kind::parse_unterminated_map:
        return "Unterminated map";
      case kind::parse_unterminated_set:
        return "Unterminated set";
      case kind::parse_odd_entries_in_map:
        return "Odd number of entries in map";
      case kind::parse_invalid_quote:
        return "Invalid quote";
      case kind::parse_invalid_meta_hint_value:
        return "Meta hint must be a keyword or map";
      case kind::parse_invalid_meta_hint_target:
        return "Invalid meta hint target";
      case kind::parse_unsupported_reader_macro:
        return "Unsupported reader macro";
      case kind::parse_nested_shorthand_function:
        return "Nested #() forms are not allowed";
      case kind::parse_invalid_shorthand_function:
        return "Invalid shorthand function";
      case kind::parse_invalid_shorthand_function_parameter:
        return "Invalid shorthand function parameter";
      case kind::parse_invalid_reader_var:
        return "Invalid reader var reference";
      case kind::parse_invalid_reader_comment:
        return "Invalid reader comment";
      case kind::parse_invalid_reader_conditional:
        return "Invalid reader conditional";
      case kind::parse_invalid_reader_splice:
        return "Invalid reader splice";
      case kind::parse_invalid_reader_gensym:
        return "gensym literal is not within a syntax quote";
      case kind::parse_invalid_reader_symbolic_value:
        return "Invalid reader symbolic value";
      case kind::parse_invalid_syntax_quote:
        return "Invalid syntax quote";
      case kind::parse_invalid_syntax_unquote:
        return "Invalid syntax unquote";
      case kind::parse_invalid_syntax_unquote_splice:
        return "Unquote splice is not within a sequence";
      case kind::parse_invalid_reader_deref:
        return "Invalid reader deref";
      case kind::parse_invalid_ratio:
        return "Invalid ratio";
      case kind::parse_invalid_keyword:
        return "Invalid keyword";
      case kind::internal_parse_failure:
        return "Internal parse failure";

      case kind::analysis_invalid_case:
        return "Invalid case";
      case kind::analysis_invalid_def:
        return "Invalid def";
      case kind::analysis_invalid_fn:
        return "Invalid fn";
      case kind::analysis_invalid_fn_parameters:
        return "Invalid fn parameters";
      case kind::analysis_invalid_recur_position:
        return "recur must be used from tail position";
      case kind::analysis_invalid_recur_from_try:
        return "recur may not be used within a 'try'";
      case kind::analysis_invalid_recur_args:
        return "The argument arity passed to 'recur' doesn't match the function's arity";
      case kind::analysis_invalid_let:
        return "Invalid let";
      case kind::analysis_invalid_loop:
        return "Invalid loop";
      case kind::analysis_invalid_if:
        return "Invalid if";
      case kind::analysis_invalid_quote:
        return "Invalid quote";
      case kind::analysis_invalid_var_reference:
        return "Invalid var reference";
      case kind::analysis_invalid_throw:
        return "Invalid throw";
      case kind::analysis_invalid_try:
        return "Invalid try";
      case kind::analysis_unresolved_var:
        return "Unresolved var";
      case kind::analysis_unresolved_symbol:
        return "Unresolved symbol";
      case kind::internal_analysis_failure:
        return "Internal analysis failure";

      case kind::internal_codegen_failure:
        return "Internal codegen failure";
      case kind::internal_runtime_failure:
        return "Internal runtime failure";
      case kind::internal_failure:
        return "Internal failure";
    }
    return "Unknown error 😮";
  }

  base::base(enum kind const k, read::source const &source)
    : kind{ k }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{{ message, source }}
  {
  }

  base::base(enum kind const k, read::source const &source, native_vector<note> const &notes)
    : kind{ k }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ notes }
  {
  }

  base::base(enum kind const k, native_persistent_string const &message, read::source const &source)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{{ message, source }}
  {
  }

  base::base(enum kind const k,
             read::source const &source,
             native_persistent_string const &note_message)
    : kind{ k }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{{ note_message, source }}
  {
  }

  base::base(enum kind const k,
             native_persistent_string const &message,
             read::source const &source,
             native_persistent_string const &note_message)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{{ note_message, source }}
  {
  }

  base::base(enum kind const k, read::source const &source, note const &note)
    : kind{ k }
    , message{ kind_to_message(k) }
    , source{ source }
    , notes{ note }
  {
  }

  base::base(enum kind const k,
             native_persistent_string const &message,
             read::source const &source,
             note const &note)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{ note }
  {
  }

  base::base(enum kind const k,
             native_persistent_string const &message,
             read::source const &source,
             native_vector<note> const &notes)
    : kind{ k }
    , message{ message }
    , source{ source }
    , notes{ notes }
  {
  }

  native_bool base::operator==(base const &rhs) const
  {
    return !(*this != rhs);
  }

  native_bool base::operator!=(base const &rhs) const
  {
    return kind != rhs.kind || source != rhs.source || message != rhs.message;
  }

  std::ostream &operator<<(std::ostream &os, base const &e)
  {
    return os << "error(" << kind_str(e.kind) << " - " << e.source << ", \"" << e.message << "\")";
  }
}

namespace jank
{
  error_ptr make_error(error::kind const kind, native_persistent_string const &message)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    return runtime::make_box<error::base>(kind,
                                          message,
                                          read::source{ runtime::to_string(file), {}, {} });
  }

  error_ptr make_error(error::kind const kind, read::source const &source)
  {
    return runtime::make_box<error::base>(kind, source);
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source const &source)
  {
    return runtime::make_box<error::base>(kind, message, source);
  }

  error_ptr
  make_error(error::kind const kind, read::source const &source, error::note const &error_note)
  {
    return runtime::make_box<error::base>(kind, source, error_note);
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source const &source,
                       error::note const &error_note)
  {
    return runtime::make_box<error::base>(kind, message, source, error_note);
  }

  error_ptr make_error(error::kind const kind,
                       read::source const &source,
                       native_persistent_string const &error_note_message)
  {
    return runtime::make_box<error::base>(kind, source, error_note_message);
  }

  error_ptr make_error(error::kind const kind,
                       read::source const &source,
                       native_vector<error::note> const &notes)
  {
    return runtime::make_box<error::base>(kind, source, notes);
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source const &source,
                       native_persistent_string const &error_note_message)
  {
    return runtime::make_box<error::base>(kind, message, source, error_note_message);
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    return runtime::make_box<error::base>(kind,
                                          message,
                                          read::source{ runtime::to_string(file), start, start });
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start,
                       read::source_position const &end)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    return runtime::make_box<error::base>(kind,
                                          message,
                                          read::source{ runtime::to_string(file), start, end });
  }

  read::source meta_source(option<runtime::object_ptr> const &o)
  {
    using namespace jank::runtime;

    auto const meta(runtime::meta(o.unwrap_or(obj::nil::nil_const())));
    auto const source(get(meta, __rt_ctx->intern_keyword("jank/source").expect_ok()));
    if(source == obj::nil::nil_const())
    {
      return read::source::unknown;
    }

    auto const file(get(source, __rt_ctx->intern_keyword("file").expect_ok()));
    auto const start(get(source, __rt_ctx->intern_keyword("start").expect_ok()));
    auto const end(get(source, __rt_ctx->intern_keyword("end").expect_ok()));

    auto const start_offset(get(start, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const start_line(get(start, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const start_col(get(start, __rt_ctx->intern_keyword("col").expect_ok()));

    auto const end_offset(get(end, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const end_line(get(end, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const end_col(get(end, __rt_ctx->intern_keyword("col").expect_ok()));

    return {
      to_string(file),
      { static_cast<size_t>(to_int(start_offset)),
              static_cast<size_t>(to_int(start_line)),
              static_cast<size_t>(to_int(start_col)) },
      {   static_cast<size_t>(to_int(end_offset)),
              static_cast<size_t>(to_int(end_line)),
              static_cast<size_t>(to_int(end_col))  }
    };
  }

}
