#pragma once

#include <jank/result.hpp>
#include <jank/option.hpp>
#include <jank/read/lex.hpp>

namespace jank::runtime
{
  struct context;
}

/* TODO: Rename file to processor. */
namespace jank::read::parse
{
  struct processor
  {
    /* TODO: none instead of nullptr. */
    using object_result = result<runtime::object_ptr, error>;

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

    processor(runtime::context &rt_ctx,
              lex::processor::iterator const &b,
              lex::processor::iterator const &e);

    object_result next();
    object_result parse_list();
    object_result parse_vector();
    object_result parse_map();
    object_result parse_quote();
    object_result parse_meta_hint();
    object_result parse_reader_macro();
    object_result parse_reader_macro_set();
    object_result parse_reader_macro_comment();
    object_result parse_symbol();
    object_result parse_nil();
    object_result parse_boolean();
    object_result parse_keyword();
    object_result parse_integer();
    object_result parse_real();
    object_result parse_string();
    object_result parse_escaped_string();

    iterator begin();
    iterator end();

    runtime::context &rt_ctx;
    lex::processor::iterator token_current, token_end;
    option<lex::token_kind> expected_closer;
    /* Whether or not the next form is considered quoted. */
    native_bool quoted{};
  };
}
