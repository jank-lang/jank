#pragma once

#include <jtl/ptr.hpp>

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/read/source.hpp>

/* NOLINTNEXTLINE(modernize-concat-nested-namespaces): Not doable, due to the inline. */
namespace cpptrace
{
  inline namespace v1
  {
    struct raw_trace;
  }
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
    lex_unexpected_character,
    lex_internal_failure,

    parse_unexpected_closing_character,
    parse_unterminated_list,
    parse_unterminated_vector,
    parse_unterminated_map,
    parse_unterminated_set,
    parse_invalid_reader_conditional,
    parse_invalid_reader_splice,
    parse_unsupported_reader_macro,
    parse_odd_entries_in_map,
    parse_duplicate_keys_in_map,
    parse_duplicate_items_in_set,
    parse_invalid_quote,
    parse_invalid_meta_hint_value,
    parse_invalid_meta_hint_target,
    parse_invalid_unicode,
    parse_invalid_character,
    parse_invalid_string_escape,
    parse_nested_shorthand_function,
    parse_invalid_shorthand_function_parameter,
    parse_invalid_reader_var,
    parse_invalid_reader_comment,
    parse_invalid_reader_gensym,
    parse_invalid_reader_symbolic_value,
    parse_invalid_reader_tag_value,
    parse_invalid_regex,
    parse_invalid_uuid,
    parse_invalid_inst,
    parse_invalid_syntax_quote,
    parse_invalid_syntax_unquote,
    parse_invalid_syntax_unquote_splice,
    parse_invalid_reader_deref,
    parse_invalid_ratio,
    parse_invalid_keyword,
    parse_invalid_data_reader,
    parse_internal_failure,

    analyze_invalid_case,
    analyze_invalid_def,
    analyze_invalid_fn,
    analyze_invalid_fn_parameters,
    analyze_invalid_recur_position,
    analyze_invalid_recur_from_try,
    analyze_invalid_recur_args,
    analyze_invalid_let,
    analyze_invalid_letfn,
    analyze_invalid_if,
    analyze_invalid_quote,
    analyze_invalid_var_reference,
    analyze_invalid_throw,
    analyze_invalid_try,
    analyze_unresolved_var,
    analyze_unresolved_symbol,
    analyze_macro_expansion_exception,
    analyze_invalid_cpp_operator_call,
    analyze_invalid_cpp_constructor_call,
    analyze_invalid_cpp_member_call,
    analyze_invalid_cpp_function_call,
    analyze_invalid_cpp_call,
    analyze_invalid_cpp_conversion,
    analyze_invalid_cpp_symbol,
    analyze_unresolved_cpp_symbol,
    analyze_invalid_cpp_raw,
    analyze_invalid_cpp_type,
    analyze_invalid_cpp_type_position,
    analyze_invalid_cpp_dsl,
    analyze_invalid_cpp_value,
    analyze_invalid_cpp_cast,
    analyze_invalid_cpp_unsafe_cast,
    analyze_invalid_cpp_box,
    analyze_invalid_cpp_unbox,
    analyze_invalid_cpp_new,
    analyze_invalid_cpp_def,
    analyze_invalid_cpp_delete,
    analyze_invalid_cpp_member_access,
    analyze_invalid_cpp_capture,
    analyze_invalid_cpp_position,
    analyze_mismatched_if_types,
    analyze_internal_failure,

    codegen_internal_failure,

    aot_unresolved_main,
    aot_internal_failure,

    runtime_module_not_found,
    runtime_module_binary_without_source,
    runtime_unable_to_open_file,
    runtime_invalid_cpp_eval,
    runtime_unable_to_load_module,
    runtime_invalid_unbox,
    runtime_non_metadatable_value,
    runtime_invalid_referred_global_symbol,
    runtime_invalid_referred_global_rename,
    runtime_unsupported_behavior,
    runtime_static_feature_disabled,
    runtime_uncaught_exception,
    runtime_internal_failure,

    system_clang_executable_not_found,
    system_failure,

    internal_failure,
  };

  char const *kind_str(kind const k);
  bool is_internal_failure_kind(kind const k);

  /* This is a clang-tidy bug. https://github.com/llvm/llvm-project/issues/61687
   * NOLINTNEXTLINE(clang-analyzer-core.uninitialized.Assign) */
  struct note
  {
    enum class kind : u8
    {
      info,
      warning,
      error,
      /* No column info for these. */
      line_start,
      info_line = line_start,
      warning_line,
      error_line,
      line_end = error_line,
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

        case kind::info_line:
          return "info_line";
        case kind::warning_line:
          return "warning_line";
        case kind::error_line:
          return "error_line";
      }
      return "unknown";
    }

    jtl::immutable_string to_string() const;

    jtl::immutable_string message;
    read::source source;
    kind kind{ kind::error };
  };

  /* TODO: We leak the stack trace. See if we can get these errors off the GC heap. */
  struct base
  {
    static constexpr bool is_error{ true };

    base() = delete;
    base(base const &) = delete;
    base(base &&) noexcept = default;
    base(kind k, read::source const &source);
    base(kind k, read::source const &source, native_vector<note> const &notes);
    base(kind k, read::source const &source, runtime::object_ref const expansion);
    base(kind k, jtl::immutable_string const &message, read::source const &source);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref const expansion);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref const expansion,
         std::unique_ptr<cpptrace::raw_trace> trace);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         jtl::immutable_string const &note_message,
         runtime::object_ref const expansion);
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
         runtime::object_ref const expansion);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         native_vector<note> const &notes);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref const expansion,
         jtl::ref<base> cause);
    base(kind k,
         jtl::immutable_string const &message,
         read::source const &source,
         runtime::object_ref const expansion,
         jtl::ref<base> cause,
         std::unique_ptr<cpptrace::raw_trace> trace);

    bool operator==(base const &rhs) const;
    bool operator!=(base const &rhs) const;

    void sort_notes();
    jtl::ref<base> add_usage(read::source const &usage_source);
    jtl::ref<base> add_fallback_usage(read::source const &usage_source);

    kind kind{};
    jtl::immutable_string message;
    read::source source;
    native_vector<note> notes;
    jtl::ptr<base> cause;
    std::unique_ptr<cpptrace::raw_trace> trace;
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
    /* This can be used by jtl helpers which can't reach into jank but which fail. */
    [[noreturn]]
    void throw_internal_failure(jtl::immutable_string const &message);

    [[noreturn]]
    void throw_result_failure(jtl::immutable_string const &message);
  }
}
