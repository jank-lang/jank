#pragma once

#include <jank/result.hpp>
#include <jank/option.hpp>
#include <jank/read/lex.hpp>
#include <jank/runtime/object.hpp>

namespace jank::read::parse
{
  struct processor
  {
    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      /* TODO: none instead of nullptr. */
      using value_type = result<runtime::object_ptr, error>;
      using pointer = value_type*;
      using reference = value_type&;

      value_type operator *() const;
      pointer operator ->();
      iterator& operator ++();
      bool operator !=(iterator const &rhs) const;
      bool operator ==(iterator const &rhs) const;

      option<value_type> latest;
      processor &p;
    };

    processor(lex::processor::iterator const &b, lex::processor::iterator const &e)
      : token_current{ b }, token_end{ e }
    { }

    result<runtime::object_ptr, error> next();
    result<runtime::object_ptr, error> parse_list();
    result<runtime::object_ptr, error> parse_vector();
    result<runtime::object_ptr, error> parse_map();
    result<runtime::object_ptr, error> parse_quote();
    result<runtime::object_ptr, error> parse_symbol();
    result<runtime::object_ptr, error> parse_keyword();
    result<runtime::object_ptr, error> parse_integer();
    result<runtime::object_ptr, error> parse_string();

    iterator begin();
    iterator end();

    lex::processor::iterator token_current, token_end;
    option<lex::token_kind> expected_closer;
  };
}
