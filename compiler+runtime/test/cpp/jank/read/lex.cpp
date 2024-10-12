#include <unistd.h>

#include <array>
#include <iostream>

#include <jank/read/lex.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

using namespace std::string_view_literals;

namespace jank::read::lex
{
  /* This is just std::to_array, pre-C++20. */
  namespace detail
  {
    template <class T, std::size_t N, std::size_t... I>
    constexpr std::array<std::remove_cv_t<T>, N> to_array_impl(T (&a)[N], std::index_sequence<I...>)
    {
      return { { a[I]... } };
    }

    template <class T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N])
    {
      return detail::to_array_impl(a, std::make_index_sequence<N>{});
    }
  }

  constexpr std::array<token, 0> make_tokens()
  {
    return {};
  }

  template <size_t N>
  constexpr std::array<token, N> make_tokens(token const (&arr)[N])
  {
    return detail::to_array(arr);
  }

  template <size_t N>
  constexpr std::array<result<token, error>, N> make_results(result<token, error> const (&arr)[N])
  {
    return detail::to_array(arr);
  }

  template <size_t N>
  bool operator==(native_vector<result<token, error>> const &v, std::array<token, N> const &a)
  {
    if(v.size() != N)
    {
      return false;
    }
    return std::equal(v.begin(), v.end(), a.begin());
  }

  template <size_t N>
  bool operator==(native_vector<result<token, error>> const &v,
                  std::array<result<token, error>, N> const &a)
  {
    if(v.size() != N)
    {
      return false;
    }
    return std::equal(v.begin(), v.end(), a.begin());
  }

  /* This really helps with doctest comparison outputs. */
  template <typename T>
  std::ostream &operator<<(std::ostream &os, native_vector<T> const &rs)
  {
    os << "[ ";
    for(auto const &r : rs)
    {
      os << r << " ";
    }
    return os << "]";
  }

  template <typename T, size_t N>
  std::ostream &operator<<(std::ostream &os, std::array<T, N> const &rs)
  {
    os << "[ ";
    for(auto const &r : rs)
    {
      os << r << " ";
    }
    return os << "]";
  }

  TEST_SUITE("lex")
  {
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
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::comment, ""sv }
        }));
      }

      SUBCASE("Empty multi-line")
      {
        processor p{ ";\n;" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::comment, ""sv },
                { 2, 1, token_kind::comment, ""sv }
        }));
      }

      SUBCASE("Non-empty")
      {
        processor p{ "; Hello hello" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 12, token_kind::comment, " Hello hello"sv }
        }));
      }

      SUBCASE("Multiple on same line")
      {
        processor p{ "; Hello hello ; \"hi hi\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 22, token_kind::comment, " Hello hello ; \"hi hi\""sv }
        }));
      }

      SUBCASE("Multiple ; in a row")
      {
        processor p{ ";;; Hello hello 12" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 17, token_kind::comment, " Hello hello 12"sv }
        }));
      }

      SUBCASE("Expressions before")
      {
        processor p{ "1 2 ; meow" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::integer,       1ll },
                { 2, 1, token_kind::integer,       2ll },
                { 4, 5, token_kind::comment, " meow"sv }
        }));
      }

      SUBCASE("Expressions before and after")
      {
        processor p{ "1 ; meow\n2" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::integer,       1ll },
                { 2, 5, token_kind::comment, " meow"sv },
                { 9, 1, token_kind::integer,       2ll }
        }));
      }
    }

    TEST_CASE("List")
    {
      SUBCASE("Empty")
      {
        processor p{ "()" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0,  token_kind::open_paren },
                { 1, token_kind::close_paren }
        }));
      }

      SUBCASE("Nested")
      {
        processor p{ "(())" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0,  token_kind::open_paren },
                { 1,  token_kind::open_paren },
                { 2, token_kind::close_paren },
                { 3, token_kind::close_paren }
        }));
      }

      SUBCASE("Unbalanced")
      {
        SUBCASE("Open")
        {
          processor p{ "(" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, token_kind::open_paren }
          }));
        }

        SUBCASE("Closed")
        {
          processor p{ ")" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, token_kind::close_paren }
          }));
        }
      }
    }

    TEST_CASE("Vector")
    {
      SUBCASE("Empty")
      {
        processor p{ "[]" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0,  token_kind::open_square_bracket },
                { 1, token_kind::close_square_bracket }
        }));
      }

      SUBCASE("Nested")
      {
        processor p{ "[[]]" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0,  token_kind::open_square_bracket },
                { 1,  token_kind::open_square_bracket },
                { 2, token_kind::close_square_bracket },
                { 3, token_kind::close_square_bracket }
        }));
      }

      SUBCASE("Mixed")
      {
        processor p{ "[(foo [1 2]) 2]" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, token_kind::open_square_bracket },
                { 1, token_kind::open_paren },
                { 2, 3, token_kind::symbol, "foo"sv },
                { 6, token_kind::open_square_bracket },
                { 7, token_kind::integer, 1ll },
                { 9, token_kind::integer, 2ll },
                { 10, token_kind::close_square_bracket },
                { 11, token_kind::close_paren },
                { 13, token_kind::integer, 2ll },
                { 14, token_kind::close_square_bracket },
        }));
      }

      SUBCASE("Unbalanced")
      {
        SUBCASE("Open")
        {
          processor p{ "[" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, token_kind::open_square_bracket }
          }));
        }

        SUBCASE("Closed")
        {
          processor p{ "]" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, token_kind::close_square_bracket }
          }));
        }
      }
    }

    TEST_CASE("Nil")
    {
      SUBCASE("Full match")
      {
        processor p{ "nil" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::nil }
        }));
      }

      SUBCASE("Partial match, prefix")
      {
        processor p{ "nili" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::symbol, "nili"sv }
        }));
      }

      SUBCASE("Partial match, suffix")
      {
        processor p{ "onil" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::symbol, "onil"sv }
        }));
      }
    }

    TEST_CASE("Boolean")
    {
      SUBCASE("Full match")
      {
        processor p{ "true false" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::boolean,  true },
                { 5, 5, token_kind::boolean, false }
        }));
      }

      SUBCASE("Partial match, prefix")
      {
        processor p{ "true- falsey" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::symbol,  "true-"sv },
                { 6, 6, token_kind::symbol, "falsey"sv }
        }));
      }

      SUBCASE("Partial match, suffix")
      {
        processor p{ "sotrue ffalse" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 6, token_kind::symbol, "sotrue"sv },
                { 7, 6, token_kind::symbol, "ffalse"sv }
        }));
      }
    }

    TEST_CASE("Integer")
    {
      SUBCASE("Positive single-char")
      {
        processor p{ "0" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, token_kind::integer, 0ll }
        }));
      }

      SUBCASE("Positive multi-char")
      {
        processor p{ "1234" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::integer, 1234ll }
        }));
      }

      SUBCASE("Negative single-char")
      {
        processor p{ "-1" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::integer, -1ll }
        }));
      }

      SUBCASE("Negative multi-char")
      {
        processor p{ "-1234" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::integer, -1234ll }
        }));
      }

      SUBCASE("Expect space")
      {
        SUBCASE("Required")
        {
          processor p{ "1234abc" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 4, token_kind::integer, 1234ll },
                  error{ 4, "expected whitespace before next token" },
                  token{ 4, 3, token_kind::symbol, "abc"sv },
          }));
        }

        SUBCASE("Not required")
        {
          processor p{ "(1234)" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, token_kind::open_paren },
                  token{ 1, 4, token_kind::integer, 1234ll },
                  token{ 5, token_kind::close_paren },
          }));
        }
      }
    }

    TEST_CASE("Real")
    {
      SUBCASE("Positive 0.")
      {
        processor p{ "0." };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::real, 0.0l }
        }));
      }

      SUBCASE("Positive 0.0")
      {
        processor p{ "0.0" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::real, 0.0l }
        }));
      }

      SUBCASE("Negative 1.")
      {
        processor p{ "-1." };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::real, -1.0l }
        }));
      }

      SUBCASE("Negative 1.5")
      {
        processor p{ "-1.5" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::real, -1.5l }
        }));
      }

      SUBCASE("Negative multi-char")
      {
        processor p{ "-1234.1234" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 10, token_kind::real, -1234.1234l }
        }));
      }

      SUBCASE("Positive no leading digit")
      {
        processor p{ ".0" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "unexpected character: ." },
                token{ 1, token_kind::integer, 0ll },
        }));
      }

      SUBCASE("Negative no leading digit")
      {
        processor p{ "-.0" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 1, "invalid number" },
                error{ 1, "unexpected character: ." },
                token{ 2, token_kind::integer, 0ll },
        }));
      }

      SUBCASE("Too many dots")
      {
        {
          processor p{ "0.0." };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 3, "invalid number" },
                  error{ 3, "unexpected character: ." },
          }));
        }

        {
          processor p{ "0..0" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 2, "invalid number" },
                  error{ 2, "unexpected character: ." },
                  token{ 3, token_kind::integer, 0ll },
          }));
        }
        {
          processor p{ "0.0.0" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 3, "invalid number" },
                  error{ 3, "unexpected character: ." },
                  token{ 4, token_kind::integer, 0ll },
          }));
        }
      }

      SUBCASE("Expect space")
      {
        SUBCASE("Required")
        {
          processor p{ "12.34abc" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 5, token_kind::real, 12.34l },
                  error{ 5, "expected whitespace before next token" },
                  token{ 5, 3, token_kind::symbol, "abc"sv },
          }));
        }

        SUBCASE("Not required")
        {
          processor p{ "(12.34)" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, token_kind::open_paren },
                  token{ 1, 5, token_kind::real, 12.34l },
                  token{ 6, token_kind::close_paren },
          }));
        }
      }
    }

    TEST_CASE("Character")
    {
      SUBCASE("Whitespace after \\")
      {
        processor p{ R"(\ )" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { error(0, "Expecting a valid character literal after \\") } }));
      }

      SUBCASE("Dangling \\")
      {
        processor p{ R"(\)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { error(0, "Expecting a valid character literal after \\") } }));
      }

      SUBCASE("Alphabetic")
      {
        processor p{ R"(\a)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::character, "a"sv }
        }));
      }

      SUBCASE("Numeric")
      {
        processor p{ R"(\1)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::character, "1"sv }
        }));
      }

      SUBCASE("Multiple characters after \\")
      {
        processor p{ R"(\11)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { error(0,
                                        3,
                                        "Invalid character literal `\\11` \nNote: Jank "
                                        "doesn't support unicode characters yet!"sv) } }));
      }

      SUBCASE("Invalid symbol after a valid char")
      {
        processor p{ R"(\1:)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 2, token_kind::character, "1"sv },
                error{ 2, "invalid keyword: expected non-whitespace character after :" }
        }));
      }

      SUBCASE("Valid consecutive characters")
      {
        processor p{ R"(\1 \newline\' \\)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 2, token_kind::character,       "1"sv },
                {  3, 8, token_kind::character, "newline"sv },
                { 11, 2, token_kind::character,       "'"sv },
                { 14, 2, token_kind::character,      "\\"sv }
        }));
      }

      SUBCASE("Character followed by a backticked keyword")
      {
        processor p{ R"(\a`:kw)" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 2, token_kind::character, "a"sv },
                token{ 2, token_kind::syntax_quote },
                token{ 3, 3, token_kind::keyword, "kw"sv }
        }));
      }
    }

    TEST_CASE("Symbol")
    {
      SUBCASE("Single-char")
      {
        processor p{ "a" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, token_kind::symbol, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ "abc" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::symbol, "abc"sv }
        }));
      }

      SUBCASE("Single slash")
      {
        processor p{ "/" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, token_kind::symbol, "/"sv }
        }));
      }

      SUBCASE("Multi slash")
      {
        processor p{ "//" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid symbol" },
        }));
      }

      SUBCASE("Starting with a slash")
      {
        processor p{ "/foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid symbol" },
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ "abc123" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 6, token_kind::symbol, "abc123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ "abc_.123/-foo+?=!&<>#%" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 22, token_kind::symbol, "abc_.123/-foo+?=!&<>#%"sv }
        }));
      }

      SUBCASE("Only -")
      {
        processor p{ "-" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, token_kind::symbol, "-"sv }
        }));
      }

      SUBCASE("Starting with -")
      {
        processor p{ "-main" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::symbol, "-main"sv }
        }));
      }

      SUBCASE("Quoted")
      {
        processor p{ "'foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, token_kind::single_quote },
                { 1, 3, token_kind::symbol, "foo"sv }
        }));
      }
    }

    TEST_CASE("Keyword")
    {
      SUBCASE("Single-char")
      {
        processor p{ ":a" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::keyword, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ ":abc" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::keyword, "abc"sv }
        }));
      }

      SUBCASE("Whitespace after :")
      {
        processor p{ ": " };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: expected non-whitespace character after :" }
        }));
      }

      SUBCASE("Single slash")
      {
        processor p{ ":/" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::keyword, "/"sv }
        }));
      }

      SUBCASE("Multi slash")
      {
        processor p{ "://" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: starts with /" },
        }));
      }

      SUBCASE("Starting with a slash")
      {
        processor p{ ":/foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: starts with /" },
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ ":abc123" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 7, token_kind::keyword, "abc123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ ":abc_.123/-foo+?=!&<>" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 21, token_kind::keyword, "abc_.123/-foo+?=!&<>"sv }
        }));
      }

      SUBCASE("Auto-resolved unqualified")
      {
        processor p{ "::foo-bar" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 9, token_kind::keyword, ":foo-bar"sv }
        }));
      }

      SUBCASE("Auto-resolved qualified")
      {
        processor p{ "::foo/bar" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 9, token_kind::keyword, ":foo/bar"sv }
        }));
      }

      SUBCASE("Too many starting colons")
      {
        processor p{ ":::foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: incorrect number of :" },
                error{ 2, "expected whitespace before next token" },
                token{ 2, 4, token_kind::keyword, "foo"sv }
        }));
      }

      SUBCASE("Way too many starting colons")
      {
        processor p{ "::::foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: incorrect number of :" },
                error{ 2, "expected whitespace before next token" },
                token{ 2, 5, token_kind::keyword, ":foo"sv }
        }));
      }
    }

    TEST_CASE("String")
    {
      SUBCASE("Empty")
      {
        processor p{ "\"\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::string, ""sv }
        }));
      }
      SUBCASE("Single-char")
      {
        processor p{ "\"a\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::string, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ "\"abc\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::string, "abc"sv }
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ "\"123\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::string, "123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ "\"and then() there was abc_123/-foo?!&<>\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 40, token_kind::string, "and then() there was abc_123/-foo?!&<>"sv }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "\"foo\nbar\nspam\t\"" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 15, token_kind::string, "foo\nbar\nspam\t"sv }
        }));
      }

      SUBCASE("With escapes")
      {
        processor p{ R"("foo\"\nbar\nspam\t\r")" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 22, token_kind::escaped_string, "foo\\\"\\nbar\\nspam\\t\\r"sv }
        }));

        processor q{ R"("\??\' \\ a\a b\b f\f v\v")" };
        native_vector<result<token, error>> tokens2(q.begin(), q.end());
        CHECK(tokens2
              == make_tokens({
                { 0, 26, token_kind::escaped_string, "\\\?\?\\' \\\\ a\\a b\\b f\\f v\\v"sv }
        }));
      }

      SUBCASE("Unterminated")
      {
        processor p{ "\"meow" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "unterminated string" },
        }));
      }
    }

    TEST_CASE("Meta hint")
    {
      SUBCASE("Empty")
      {
        processor p{ "^" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::meta_hint }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "^\n:foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::meta_hint },
                { 2, 4, token_kind::keyword, "foo"sv }
        }));
      }
    }

    TEST_CASE("Reader macro")
    {
      SUBCASE("Empty")
      {
        processor p{ "#" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::reader_macro }
        }));
      }

      SUBCASE("Comment")
      {
        SUBCASE("Empty")
        {
          processor p{ "#_" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_comment }
          }));
        }

        SUBCASE("No whitespace after")
        {
          processor p{ "#_[]" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_comment },
                  { 2, 1,  token_kind::open_square_bracket },
                  { 3, 1, token_kind::close_square_bracket }
          }));
        }
      }

      SUBCASE("Conditional")
      {
        SUBCASE("Empty")
        {
          processor p{ "#?" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_conditional }
          }));
        }

        SUBCASE("With following list")
        {
          processor p{ "#?()" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_conditional },
                  { 2, 1,               token_kind::open_paren },
                  { 3, 1,              token_kind::close_paren }
          }));
        }
      }

      SUBCASE("Set")
      {
        SUBCASE("Empty")
        {
          processor p{ "#{}" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1,        token_kind::reader_macro },
                  { 1, 1,  token_kind::open_curly_bracket },
                  { 2, 1, token_kind::close_curly_bracket }
          }));
        }

        /* Clojure doesn't actually allow this, but I don't see why not. It does for meta hints, so
         * I figure this is just a lazy inconsistency. */
        SUBCASE("With line breaks")
        {
          processor p{ "#\n{}" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1,        token_kind::reader_macro },
                  { 2, 1,  token_kind::open_curly_bracket },
                  { 3, 1, token_kind::close_curly_bracket }
          }));
        }
      }
    }

    TEST_CASE("Syntax quoting")
    {
      SUBCASE("Empty")
      {
        processor p{ "`" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::syntax_quote }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "`\n:foo" };
        native_vector<result<token, error>> tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::syntax_quote },
                { 2, 4, token_kind::keyword, "foo"sv }
        }));
      }

      SUBCASE("Unquote")
      {
        SUBCASE("Empty")
        {
          processor p{ "~" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::unquote }
          }));
        }

        SUBCASE("With line breaks")
        {
          processor p{ "~\n:foo" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::unquote },
                  { 2, 4, token_kind::keyword, "foo"sv }
          }));
        }
      }

      SUBCASE("Unquote splicing")
      {
        SUBCASE("Empty")
        {
          processor p{ "~@" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::unquote_splice }
          }));
        }

        SUBCASE("With line breaks before")
        {
          processor p{ "~\n@:foo" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::unquote },
                  { 2, 1, token_kind::deref },
                  { 3, 4, token_kind::keyword, "foo"sv }
          }));
        }

        SUBCASE("With line breaks after")
        {
          processor p{ "~@\n:foo" };
          native_vector<result<token, error>> tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::unquote_splice },
                  { 3, 4, token_kind::keyword, "foo"sv }
          }));
        }
      }
    }
  }
}
