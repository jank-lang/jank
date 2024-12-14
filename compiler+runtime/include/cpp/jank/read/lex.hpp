#pragma once

#include <boost/variant.hpp>

#include <jank/result.hpp>
#include <jank/option.hpp>

namespace jank::read::lex
{
  enum class token_kind : uint8_t
  {
    open_paren,
    close_paren,
    open_square_bracket,
    close_square_bracket,
    open_curly_bracket,
    close_curly_bracket,
    single_quote,
    meta_hint,
    reader_macro,
    reader_macro_comment,
    reader_macro_conditional,
    reader_macro_conditional_splice,
    syntax_quote,
    unquote,
    unquote_splice,
    deref,
    /* Has string data. */
    comment,
    nil,
    /* Has bool data. */
    boolean,
    /* Has string data. */
    character,
    /* Has string data. */
    symbol,
    /* Has string data. */
    keyword,
    /* Has int data. */
    integer,
    /* Has double data. */
    real,
    /* Has two integer data. */
    ratio,
    /* Has string data. */
    string,
    /* Has string data. */
    escaped_string,
    eof,
  };

  constexpr char const *token_kind_str(token_kind const kind)
  {
    switch(kind)
    {
      case token_kind::open_paren:
        return "open_paren";
      case token_kind::close_paren:
        return "close_paren";
      case token_kind::open_square_bracket:
        return "open_square_bracket";
      case token_kind::close_square_bracket:
        return "close_square_bracket";
      case token_kind::open_curly_bracket:
        return "open_curly_bracket";
      case token_kind::close_curly_bracket:
        return "close_curly_bracket";
      case token_kind::single_quote:
        return "single_quote";
      case token_kind::meta_hint:
        return "meta_hint";
      case token_kind::reader_macro:
        return "reader_macro";
      case token_kind::reader_macro_comment:
        return "reader_macro_comment";
      case token_kind::reader_macro_conditional:
        return "reader_macro_conditional";
      case token_kind::reader_macro_conditional_splice:
        return "reader_macro_conditional_splice";
      case token_kind::syntax_quote:
        return "syntax_quote";
      case token_kind::unquote:
        return "unquote";
      case token_kind::unquote_splice:
        return "unquote_splice";
      case token_kind::deref:
        return "deref";
      case token_kind::comment:
        return "comment";
      case token_kind::nil:
        return "nil";
      case token_kind::boolean:
        return "boolean";
      case token_kind::character:
        return "character";
      case token_kind::symbol:
        return "symbol";
      case token_kind::keyword:
        return "keyword";
      case token_kind::integer:
        return "integer";
      case token_kind::real:
        return "real";
      case token_kind::ratio:
        return "ratio";
      case token_kind::string:
        return "string";
      case token_kind::escaped_string:
        return "escaped_string";
      case token_kind::eof:
        return "eof";
    }
  }

  struct ratio
  {
    native_integer numerator{};
    native_integer denominator{};
    native_bool operator==(ratio const &rhs) const;
    native_bool operator!=(ratio const &rhs) const;
  };

  struct token
  {
    token() = default;
    token(token_kind const k);
    token(size_t const p, token_kind const k);
    token(size_t const p, token_kind const k, native_integer const);
    token(size_t const p, token_kind const k, native_real const);
    token(size_t const p, token_kind const k, native_persistent_string_view const);
    token(size_t const p, token_kind const k, native_bool const);

    token(size_t const p, size_t const s, token_kind const k);
    token(size_t const p, size_t const s, token_kind const k, native_integer const);
    token(size_t const p, size_t const s, token_kind const k, native_real const);
    token(size_t const p, size_t const s, token_kind const k, native_persistent_string_view const);
    token(size_t const p, size_t const s, token_kind const k, char const * const);
    token(size_t const p, size_t const s, token_kind const k, native_bool const);
    token(size_t const p, size_t const s, token_kind const k, ratio const);

    native_bool operator==(token const &rhs) const;
    native_bool operator!=(token const &rhs) const;

    struct no_data
    {
      native_bool operator==(no_data const &rhs) const;
      native_bool operator!=(no_data const &rhs) const;
    };

    /* For some values, when comparing tokens, the position doesn't matter.
     * For example, EOF tokens. In these cases, this cardinal value is used. */
    static constexpr size_t ignore_pos{ std::numeric_limits<size_t>::max() };
    size_t pos{ ignore_pos };
    size_t size{ 1 };
    token_kind kind{ token_kind::eof };
    boost::variant<no_data,
                   native_integer,
                   native_real,
                   native_persistent_string_view,
                   native_bool,
                   ratio>
      data;
  };

  std::ostream &operator<<(std::ostream &os, token const &t);
  std::ostream &operator<<(std::ostream &os, token::no_data const &t);
  std::ostream &operator<<(std::ostream &os, ratio const &r);
}

namespace jank::read
{
  struct error
  {
    error(size_t const s, native_persistent_string const &m);
    error(size_t const s, size_t const e, native_persistent_string const &m);
    error(native_persistent_string const &m);

    native_bool operator==(error const &rhs) const;
    native_bool operator!=(error const &rhs) const;

    size_t start{};
    size_t end{};
    native_persistent_string message;
  };

  std::ostream &operator<<(std::ostream &os, error const &e);
}

namespace jank::read::lex
{
  struct codepoint;

  struct processor
  {
    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = result<token, error>;
      using pointer = value_type *;
      using reference = value_type &;

      value_type const &operator*() const;
      value_type const *operator->() const;
      iterator &operator++();
      native_bool operator==(iterator const &rhs) const;
      native_bool operator!=(iterator const &rhs) const;

      option<value_type> latest;
      processor &p;
    };

    processor(native_persistent_string_view const &f);

    result<token, error> next();
    result<codepoint, error> peek() const;
    option<error> check_whitespace(native_bool const found_space);

    iterator begin();
    iterator end();

    size_t pos{};
    /* Whether or not the previous token requires a space after it. */
    native_bool require_space{};
    /* True when seeing a '/' following a number. */
    native_bool found_slash_after_number{};
    native_persistent_string_view file;
  };
}
