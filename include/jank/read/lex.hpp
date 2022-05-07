#pragma once

#include <string_view>
#include <variant>

#include <jank/detail/type.hpp>
#include <jank/result.hpp>
#include <jank/option.hpp>

namespace jank::read::lex
{
  enum class token_kind
  {
    open_paren,
    close_paren,
    open_square_bracket,
    close_square_bracket,
    open_curly_bracket,
    close_curly_bracket,
    /* Has string data. */
    symbol,
    /* Has string data. */
    keyword,
    /* Has int data. */
    integer,
    /* Has string data. */
    string,
    eof,
  };
  struct token
  {
    token(token_kind const k) : kind{ k }
    { }
    template <typename T>
    token(token_kind const k, T &&t) : kind{ k }, data{ std::forward<T>(t) }
    { }

    bool operator ==(token const &rhs) const;
    bool operator !=(token const &rhs) const;

    struct no_data
    {
      bool operator ==(no_data const &rhs) const;
      bool operator !=(no_data const &rhs) const;
    };

    token_kind kind;
    std::variant<no_data, detail::int_type, std::string_view> data;
  };
  std::ostream& operator <<(std::ostream &os, token const &t);
  std::ostream& operator <<(std::ostream &os, token::no_data const &t);

  struct error
  {
    bool operator ==(error const &rhs) const
    { return pos == rhs.pos && message == rhs.message; }
    bool operator !=(error const &rhs) const
    { return pos != rhs.pos || message != rhs.message; }

    size_t pos{};
    detail::string_type message;
  };
  std::ostream& operator <<(std::ostream &os, error const &e);

  struct processor;
  struct token_iterator
  {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = result<token, error>;
    using pointer = value_type*;
    using reference = value_type&;

    value_type operator *() const;
    token_iterator& operator ++();
    bool operator !=(token_iterator const &rhs) const;

    option<value_type> latest;
    processor &p;
  };

  struct processor
  {
    processor(std::string_view const &f) : file{ f }
    { }

    result<token, error> next();
    option<char> peek() const;
    option<error> check_whitespace(bool const found_space);

    token_iterator begin();
    token_iterator end();

    size_t pos{};
    /* Whether or not the previous token requires a space after it. */
    bool require_space{};
    std::string_view file;
  };
}
