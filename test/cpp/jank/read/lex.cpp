#include <unistd.h>

#include <vector>
#include <array>
#include <iostream>

#include <jank/read/lex.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

namespace jank::read::lex
{
  /* This is just std::to_array, pre-C++20. */
  namespace detail
  {
    template <class T, std::size_t N, std::size_t... I>
    constexpr std::array<std::remove_cv_t<T>, N> to_array_impl(T (&a)[N], std::index_sequence<I...>)
    { return { {a[I]...} }; }

    template <class T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N])
    { return detail::to_array_impl(a, std::make_index_sequence<N>{}); }
  }

  constexpr std::array<token, 0> make_tokens()
  { return {}; }
  template <size_t N>
  constexpr std::array<token, N> make_tokens(token const (&arr)[N])
  { return detail::to_array(arr); }
  template <size_t N>
  constexpr std::array<result<token, error>, N> make_results(result<token, error> const (&arr)[N])
  { return detail::to_array(arr); }

  template <size_t N>
  bool operator ==(std::vector<result<token, error>> const &v, std::array<token, N> const &a)
  {
    if(v.size() != N)
    { return false; }
    return std::equal(v.begin(), v.end(), a.begin());
  }
  template <size_t N>
  bool operator ==(std::vector<result<token, error>> const &v, std::array<result<token, error>, N> const &a)
  {
    if(v.size() != N)
    { return false; }
    return std::equal(v.begin(), v.end(), a.begin());
  }

  /* This really helps with doctest comparison outputs. */
  template <typename T>
  std::ostream& operator<<(std::ostream &os, std::vector<T> const &rs)
  {
    os << "[ ";
    for(auto const &r : rs)
    { os << r << " "; }
    return os << "]";
  }
  template <typename T, size_t N>
  std::ostream& operator<<(std::ostream &os, std::array<T, N> const &rs)
  {
    os << "[ ";
    for(auto const &r : rs)
    { os << r << " "; }
    return os << "]";
  }

  TEST_CASE("Empty")
  {
    processor p{ "" };
    auto r(p.next());
    CHECK(r.is_ok());
    auto const t(r.unwrap_move());
    CHECK(t.kind == token_kind::eof);
  }

  TEST_CASE("Whitespace")
  {
    processor p{ "  ,,,,,  ," };
    auto r(p.next());
    CHECK(r.is_ok());
    auto const t(r.unwrap_move());
    CHECK(t.kind == token_kind::eof);
  }

  TEST_CASE("Comments")
  {
    SUBCASE("Empty")
    {
      processor p{ ";" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens());
    }

    SUBCASE("Non-empty")
    {
      processor p{ "; Hello hello" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens());
    }

    SUBCASE("Multiple on same line")
    {
      processor p{ "; Hello hello ; \"hi hi\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens());
    }

    SUBCASE("Multiple ; in a row")
    {
      processor p{ ";;; Hello hello 12" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens());
    }

    SUBCASE("Expressions before")
    {
      processor p{ "1 2 ; meow" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({ { 0, token_kind::integer, 1ll }, { 2, token_kind::integer, 2ll } }));
    }

    SUBCASE("Expressions before and after")
    {
      processor p{ "1 ; meow\n2" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({ { 0, token_kind::integer, 1ll }, { 9, token_kind::integer, 2ll } }));
    }
  }

  TEST_CASE("List")
  {
    SUBCASE("Empty")
    {
      processor p{ "()" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::open_paren }, { 1, token_kind::close_paren }}));
    }

    SUBCASE("Nested")
    {
      processor p{ "(())" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens
      (
        {
          { 0, token_kind::open_paren }, { 1, token_kind::open_paren },
          { 2, token_kind::close_paren }, { 3, token_kind::close_paren }
        }
      ));
    }

    SUBCASE("Unbalanced")
    {
      SUBCASE("Open")
      {
        processor p{ "(" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_tokens({{ 0, token_kind::open_paren }}));
      }

      SUBCASE("Closed")
      {
        processor p{ ")" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_tokens({{ 0, token_kind::close_paren }}));
      }
    }
  }

  TEST_CASE("Vector")
  {
    SUBCASE("Empty")
    {
      processor p{ "[]" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::open_square_bracket }, { 1, token_kind::close_square_bracket }}));
    }

    SUBCASE("Nested")
    {
      processor p{ "[[]]" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens
      (
        {
          { 0, token_kind::open_square_bracket }, { 1, token_kind::open_square_bracket },
          { 2, token_kind::close_square_bracket }, { 3, token_kind::close_square_bracket }
        }
      ));
    }

    SUBCASE("Mixed")
    {
      processor p{ "[(foo [1 2]) 2]" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens
      (
        {
          { 0, token_kind::open_square_bracket },
          { 1, token_kind::open_paren },
          { 2, 3, token_kind::symbol, "foo" },
          { 6, token_kind::open_square_bracket },
          { 7, token_kind::integer, 1ll },
          { 9, token_kind::integer, 2ll },
          { 10, token_kind::close_square_bracket },
          { 11, token_kind::close_paren },
          { 13, token_kind::integer, 2ll },
          { 14, token_kind::close_square_bracket },
        }
      ));
    }

    SUBCASE("Unbalanced")
    {
      SUBCASE("Open")
      {
        processor p{ "[" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_tokens({{ 0, token_kind::open_square_bracket }}));
      }

      SUBCASE("Closed")
      {
        processor p{ "]" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_tokens({{ 0, token_kind::close_square_bracket }}));
      }
    }
  }

  TEST_CASE("Nil")
  {
    SUBCASE("Full match")
    {
      processor p{ "nil" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 3, token_kind::nil }}));
    }

    SUBCASE("Partial match, prefix")
    {
      processor p{ "nili" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 4, token_kind::symbol, "nili" }}));
    }

    SUBCASE("Partial match, suffix")
    {
      processor p{ "onil" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 4, token_kind::symbol, "onil" }}));
    }
  }

  TEST_CASE("Integer")
  {
    SUBCASE("Positive single-char")
    {
      processor p{ "0" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::integer, 0ll }}));
    }

    SUBCASE("Positive multi-char")
    {
      processor p{ "1234" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 4, token_kind::integer, 1234ll }}));
    }

    SUBCASE("Negative single-char")
    {
      processor p{ "-1" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 2, token_kind::integer, -1ll }}));
    }

    SUBCASE("Negative multi-char")
    {
      processor p{ "-1234" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 5, token_kind::integer, -1234ll }}));
    }

    SUBCASE("Expect space")
    {
      SUBCASE("Required")
      {
        processor p{ "1234abc" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            token{ 0, 4, token_kind::integer, 1234ll },
            error{ 4, "expected whitespace before next token" },
            token{ 4, 3, token_kind::symbol, "abc" },
          }
        ));
      }

      SUBCASE("Not required")
      {
        processor p{ "(1234)" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            token{ 0, token_kind::open_paren },
            token{ 1, 4, token_kind::integer, 1234ll },
            token{ 5, token_kind::close_paren },
          }
        ));
      }
    }
  }

  TEST_CASE("Real")
  {
    SUBCASE("Positive 0.")
    {
      processor p{ "0." };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 2, token_kind::real, 0.0l }}));
    }

    SUBCASE("Positive 0.0")
    {
      processor p{ "0.0" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 3, token_kind::real, 0.0l }}));
    }

    SUBCASE("Negative 1.")
    {
      processor p{ "-1." };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 3, token_kind::real, -1.0l }}));
    }

    SUBCASE("Negative 1.5")
    {
      processor p{ "-1.5" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 4, token_kind::real, -1.5l }}));
    }

    SUBCASE("Negative multi-char")
    {
      processor p{ "-1234.1234" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 10, token_kind::real, -1234.1234l }}));
    }

    SUBCASE("Positive no leading digit")
    {
      processor p{ ".0" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results
      (
        {
          error{ 0, "unexpected character: ." },
          token{ 1, token_kind::integer, 0ll },
        }
      ));
    }

    SUBCASE("Negative no leading digit")
    {
      processor p{ "-.0" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results
      (
        {
          error{ 0, 1, "invalid number" },
          error{ 1, "unexpected character: ." },
          token{ 2, token_kind::integer, 0ll },
        }
      ));
    }

    SUBCASE("Too many dots")
    {
      {
        processor p{ "0.0." };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            error{ 0, 3, "invalid number" },
            error{ 3, "unexpected character: ." },
          }
        ));
      }

      {
        processor p{ "0..0" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            error{ 0, 2, "invalid number" },
            error{ 2, "unexpected character: ." },
            token{ 3, token_kind::integer, 0ll },
          }
        ));
      }
      {
        processor p{ "0.0.0" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            error{ 0, 3, "invalid number" },
            error{ 3, "unexpected character: ." },
            token{ 4, token_kind::integer, 0ll },
          }
        ));
      }
    }

    SUBCASE("Expect space")
    {
      SUBCASE("Required")
      {
        processor p{ "12.34abc" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            token{ 0, 5, token_kind::real, 12.34l },
            error{ 5, "expected whitespace before next token" },
            token{ 5, 3, token_kind::symbol, "abc" },
          }
        ));
      }

      SUBCASE("Not required")
      {
        processor p{ "(12.34)" };
        std::vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens == make_results
        (
          {
            token{ 0, token_kind::open_paren },
            token{ 1, 5, token_kind::real, 12.34l },
            token{ 6, token_kind::close_paren },
          }
        ));
      }
    }
  }

  TEST_CASE("Symbol")
  {
    SUBCASE("Single-char")
    {
      processor p{ "a" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::symbol, "a" }}));
    }

    SUBCASE("Multi-char")
    {
      processor p{ "abc" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 3, token_kind::symbol, "abc" }}));
    }

    SUBCASE("Single slash")
    {
      processor p{ "/" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::symbol, "/" }}));
    }

    SUBCASE("Multi slash")
    {
      processor p{ "//" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results({ error{ 0, "invalid symbol" }, }));
    }

    SUBCASE("Starting with a slash")
    {
      processor p{ "/foo" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results({ error{ 0, "invalid symbol" }, }));
    }

    SUBCASE("With numbers")
    {
      processor p{ "abc123" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 6, token_kind::symbol, "abc123" }}));
    }

    SUBCASE("With other symbols")
    {
      processor p{ "abc_.123/-foo+?" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 15, token_kind::symbol, "abc_.123/-foo+?" }}));
    }

    SUBCASE("Only -")
    {
      processor p{ "-" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::symbol, "-" }}));
    }

    SUBCASE("Starting with -")
    {
      processor p{ "-main" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 5, token_kind::symbol, "-main" }}));
    }

    SUBCASE("Quoted")
    {
      processor p{ "'foo" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, token_kind::single_quote }, { 1, 3, token_kind::symbol, "foo" }}));
    }
  }

  TEST_CASE("Keyword")
  {
    SUBCASE("Single-char")
    {
      processor p{ ":a" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 2, token_kind::keyword, "a" }}));
    }

    SUBCASE("Multi-char")
    {
      processor p{ ":abc" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 4, token_kind::keyword, "abc" }}));
    }

    SUBCASE("Single slash")
    {
      processor p{ ":/" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 2, token_kind::keyword, "/" }}));
    }

    SUBCASE("Multi slash")
    {
      processor p{ "://" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results({ error{ 0, "invalid keyword" }, }));
    }

    SUBCASE("Starting with a slash")
    {
      processor p{ ":/foo" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results({ error{ 0, "invalid keyword" }, }));
    }

    SUBCASE("With numbers")
    {
      processor p{ ":abc123" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 7, token_kind::keyword, "abc123" }}));
    }

    SUBCASE("With other symbols")
    {
      processor p{ ":abc_.123/-foo+?" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 16, token_kind::keyword, "abc_.123/-foo+?" }}));
    }

    SUBCASE("Auto-resolved unqualified")
    {
      processor p{ "::foo-bar" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 9, token_kind::keyword, ":foo-bar" }}));
    }

    SUBCASE("Auto-resolved qualified")
    {
      processor p{ "::foo/bar" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 9, token_kind::keyword, ":foo/bar" }}));
    }

    SUBCASE("Too many starting colons")
    {
      processor p{ ":::foo" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results
      (
        {
          error{ 0, "invalid keyword" },
          error{ 2, "expected whitespace before next token" },
          token{ 2, 4, token_kind::keyword, "foo" }
        }
      ));
    }

    SUBCASE("Way too many starting colons")
    {
      processor p{ "::::foo" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results
      (
        {
          error{ 0, "invalid keyword" },
          error{ 2, "expected whitespace before next token" },
          token{ 2, 5, token_kind::keyword, ":foo" }
        }
      ));
    }
  }

  TEST_CASE("String")
  {
    SUBCASE("Empty")
    {
      processor p{ "\"\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 2, token_kind::string, "" }}));
    }
    SUBCASE("Single-char")
    {
      processor p{ "\"a\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 3, token_kind::string, "a" }}));
    }

    SUBCASE("Multi-char")
    {
      processor p{ "\"abc\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 5, token_kind::string, "abc" }}));
    }

    SUBCASE("With numbers")
    {
      processor p{ "\"123\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 5, token_kind::string, "123" }}));
    }

    SUBCASE("With other symbols")
    {
      processor p{ "\"and then() there was abc_123/-foo?\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 36, token_kind::string, "and then() there was abc_123/-foo?" }}));
    }

    SUBCASE("With line breaks")
    {
      processor p{ "\"foo\nbar\nspam\t\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_tokens({{ 0, 15, token_kind::string, "foo\nbar\nspam\t" }}));
    }

    /* TODO: Figure out how to handle this. */
    SUBCASE("With escapes")
    {
      processor p{ "\"foo\\\"\\nbar\\nspam\\t\"" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      WARN(tokens == make_tokens({{ 0, token_kind::string, "foo\"\nbar\nspam\t" }}));
    }

    SUBCASE("Unterminated")
    {
      processor p{ "\"meow" };
      std::vector<result<token, error>> tokens(p.begin(), p.end());
      CHECK(tokens == make_results({ error{ 0, "unterminated string" }, }));
    }
  }
}
