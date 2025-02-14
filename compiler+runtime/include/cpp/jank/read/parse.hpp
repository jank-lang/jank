#pragma once

#include <list>

#include <jank/result.hpp>
#include <jank/option.hpp>
#include <jank/read/lex.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  struct context;
}

/* TODO: Rename file to processor. */
namespace jank::read::parse
{
  struct char_parse_error
  {
    native_persistent_string error;
  };

  result<native_persistent_string, char_parse_error>
  parse_character_in_base(native_persistent_string const &char_literal, int const base);

  option<char> get_char_from_literal(native_persistent_string const &s);

  struct object_source_info
  {
    native_bool operator==(object_source_info const &rhs) const;
    native_bool operator!=(object_source_info const &rhs) const;

    runtime::object_ptr ptr{};
    lex::token start, end;
  };

  struct processor
  {
    using object_result = result<option<object_source_info>, error_ptr>;

    struct shorthand_function_details
    {
      option<uint8_t> max_fixed_arity{};
      native_bool variadic{};
      source source;
    };

    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = object_result;
      using pointer = value_type *;
      using reference = value_type &;

      value_type operator*() const;
      pointer operator->();
      iterator &operator++();
      native_bool operator!=(iterator const &rhs) const;
      native_bool operator==(iterator const &rhs) const;
      iterator &operator=(iterator const &);

      option<value_type> latest;
      processor &p;
    };

    processor(lex::processor::iterator const &b, lex::processor::iterator const &e);

    object_result next();
    object_result parse_list();
    object_result parse_vector();
    object_result parse_map();
    object_result parse_quote();
    object_result parse_character();
    object_result parse_meta_hint();
    object_result parse_reader_macro();
    object_result parse_reader_macro_set();
    object_result parse_reader_macro_fn();
    object_result parse_reader_macro_var_quote();
    object_result parse_reader_macro_symbolic_values();
    object_result parse_reader_macro_comment();
    object_result parse_reader_macro_conditional(native_bool splice);
    object_result parse_syntax_quote();
    object_result parse_unquote(native_bool splice);
    object_result parse_deref();
    object_result parse_symbol();
    object_result parse_nil();
    object_result parse_boolean();
    object_result parse_keyword();
    object_result parse_integer();
    object_result parse_ratio();
    object_result parse_real();
    object_result parse_string();
    object_result parse_escaped_string();

    iterator begin();
    iterator end();

  private:
    result<runtime::object_ptr, error_ptr> syntax_quote(runtime::object_ptr form);
    result<runtime::object_ptr, error_ptr> syntax_quote_expand_seq(runtime::object_ptr seq);
    static result<runtime::object_ptr, error_ptr> syntax_quote_flatten_map(runtime::object_ptr seq);
    static native_bool syntax_quote_is_unquote(runtime::object_ptr form, native_bool splice);

  public:
    lex::processor::iterator token_current, token_end;
    option<lex::token_kind> expected_closer;
    /* Splicing, in reader conditionals, is not allowed at the top level. When we're parsing
     * some other form, such as a list, we'll bind this var to true. */
    runtime::var_ptr splicing_allowed_var{};
    /* When we've spliced some forms, we'll put them into this list. Before reading the next
     * token, we should check this list to see if there's already a form we should pull out.
     * This is needed because parse iteration works one form at a time and splicing potentially
     * turns one form into many. */
    std::list<runtime::object_ptr> pending_forms;
    lex::token latest_token;
    option<shorthand_function_details> shorthand;
    /* Whether or not the next form is considered quoted. */
    native_bool quoted{};
    /* Whether or not the next form is considered syntax-quoted. */
    native_bool syntax_quoted{};
  };
}
