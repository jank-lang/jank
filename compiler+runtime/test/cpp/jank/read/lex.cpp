#include <array>
#include <ostream>

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
    static constexpr std::array<std::remove_cv_t<T>, N>
    to_array_impl(T (&a)[N], std::index_sequence<I...>)
    {
      return { { a[I]... } };
    }

    template <class T, std::size_t N>
    static constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N])
    {
      return detail::to_array_impl(a, std::make_index_sequence<N>{});
    }
  }

  static constexpr std::array<token, 0> make_tokens()
  {
    return {};
  }

  template <size_t N>
  static constexpr std::array<token, N> make_tokens(token const (&arr)[N])
  {
    return detail::to_array(arr);
  }

  template <size_t N>
  static constexpr std::array<result<token, error_ptr>, N>
  make_results(result<token, error_ptr> const (&arr)[N])
  {
    return detail::to_array(arr);
  }

  template <size_t N>
  static bool
  operator==(native_vector<result<token, error_ptr>> const &v, std::array<token, N> const &a)
  {
    if(v.size() != N)
    {
      return false;
    }

    return std::equal(v.begin(), v.end(), a.begin());
  }

  template <size_t N>
  static bool operator==(native_vector<result<token, error_ptr>> const &v,
                         std::array<result<token, error_ptr>, N> const &a)
  {
    if(v.size() != N)
    {
      return false;
    }

    for(size_t i{}; i < v.size(); ++i)
    {
      auto const &lhs(v[i]);
      auto const &rhs(a[i]);

      if(lhs.is_err())
      {
        if(rhs.is_ok())
        {
          return false;
        }

        auto const &lhs_err(lhs.expect_err());
        auto const &rhs_err(rhs.expect_err());
        return lhs_err->kind == rhs_err->kind && lhs_err->source == rhs_err->source;
      }
      else
      {
        if(rhs.is_err())
        {
          return false;
        }

        auto const &lhs_token(lhs.expect_ok());
        auto const &rhs_token(rhs.expect_ok());
        return lhs_token == rhs_token;
      }
    }
  }

  /* This really helps with doctest comparison outputs. */
  template <typename T>
  static std::ostream &operator<<(std::ostream &os, native_vector<T> const &rs)
  {
    os << "[ ";
    for(auto const &r : rs)
    {
      os << r << " ";
    }
    return os << "]";
  }

  template <typename T, size_t N>
  static std::ostream &operator<<(std::ostream &os, std::array<T, N> const &rs)
  {
    os << "[ ";
    for(auto const &r : rs)
    {
      os << r << " ";
    }
    return os << "]";
  }

  static std::ostream &operator<<(std::ostream &os, result<token, error_ptr> const &r)
  {
    if(r.is_ok())
    {
      return os << "ok(" << r.expect_ok() << ")";
    }
    else
    {
      os << "err(";
      os << error::kind_str(r.expect_err()->kind) << ", ";
      os << r.expect_err()->source;
      return os << ")";
    }
  }

  static error_ptr make_error(error::kind const kind, size_t const offset, size_t const width)
  {
    return runtime::make_box<error::base>(kind,
                                          read::source{
                                            "NO_SOURCE_PATH",
                                            {         offset, 1,         offset + 1 },
                                            { offset + width, 1, offset + width + 1 }
    });
  }

  TEST_SUITE("lex")
  {
    using namespace jank::error;

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
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::comment, ""sv }
        }));
      }

      SUBCASE("Empty multi-line")
      {
        processor p{ ";\n;" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { { 0, 1, 1 }, { 1, 1, 2 }, token_kind::comment, ""sv },
                { { 2, 2, 1 }, { 3, 2, 2 }, token_kind::comment, ""sv }
        }));
      }

      SUBCASE("Non-empty")
      {
        processor p{ "; Hello hello" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { { 0, 1, 1 }, { 13, 1, 14 }, token_kind::comment, " Hello hello"sv }
        }));
      }

      SUBCASE("Multiple on same line")
      {
        processor p{ "; Hello hello ; \"hi hi\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(
          tokens
          == make_tokens({
            { { 0, 1, 1 }, { 23, 1, 24 }, token_kind::comment, " Hello hello ; \"hi hi\""sv }
        }));
      }

      SUBCASE("Multiple ; in a row")
      {
        processor p{ ";;; Hello hello 12" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { { 0, 1, 1 }, { 18, 1, 19 }, token_kind::comment, " Hello hello 12"sv }
        }));
      }

      SUBCASE("Expressions before")
      {
        processor p{ "1 2 ; meow" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {           0,             1, token_kind::integer,       1ll },
                {           2,             1, token_kind::integer,       2ll },
                { { 4, 1, 5 }, { 10, 1, 11 }, token_kind::comment, " meow"sv }
        }));
      }

      SUBCASE("Expressions before and after")
      {
        processor p{ "1 ; meow\n2" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {           0,            1, token_kind::integer,       1ll },
                { { 2, 1, 3 },  { 8, 1, 9 }, token_kind::comment, " meow"sv },
                { { 9, 2, 1 }, { 10, 2, 2 }, token_kind::integer,       2ll }
        }));
      }
    }

    TEST_CASE("List")
    {
      SUBCASE("Empty")
      {
        processor p{ "()" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1,  token_kind::open_paren },
                { 1, 1, token_kind::close_paren }
        }));
      }

      SUBCASE("Nested")
      {
        processor p{ "(())" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1,  token_kind::open_paren },
                { 1, 1,  token_kind::open_paren },
                { 2, 1, token_kind::close_paren },
                { 3, 1, token_kind::close_paren }
        }));
      }

      SUBCASE("Unbalanced")
      {
        SUBCASE("Open")
        {
          processor p{ "(" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::open_paren }
          }));
        }

        SUBCASE("Closed")
        {
          processor p{ ")" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::close_paren }
          }));
        }
      }
    }

    TEST_CASE("Vector")
    {
      SUBCASE("Empty")
      {
        processor p{ "[]" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1,  token_kind::open_square_bracket },
                { 1, 1, token_kind::close_square_bracket }
        }));
      }

      SUBCASE("Nested")
      {
        processor p{ "[[]]" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1,  token_kind::open_square_bracket },
                { 1, 1,  token_kind::open_square_bracket },
                { 2, 1, token_kind::close_square_bracket },
                { 3, 1, token_kind::close_square_bracket }
        }));
      }

      SUBCASE("Mixed")
      {
        processor p{ "[(foo [1 2]) 2]" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::open_square_bracket },
                { 1, 1, token_kind::open_paren },
                { 2, 3, token_kind::symbol, "foo"sv },
                { 6, 1, token_kind::open_square_bracket },
                { 7, 1, token_kind::integer, 1ll },
                { 9, 1, token_kind::integer, 2ll },
                { 10, 1, token_kind::close_square_bracket },
                { 11, 1, token_kind::close_paren },
                { 13, 1, token_kind::integer, 2ll },
                { 14, 1, token_kind::close_square_bracket },
        }));
      }

      SUBCASE("Unbalanced")
      {
        SUBCASE("Open")
        {
          processor p{ "[" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::open_square_bracket }
          }));
        }

        SUBCASE("Closed")
        {
          processor p{ "]" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::close_square_bracket }
          }));
        }
      }
    }

    TEST_CASE("Nil")
    {
      SUBCASE("Full match")
      {
        processor p{ "nil" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::nil }
        }));
      }

      SUBCASE("Partial match, prefix")
      {
        processor p{ "nili" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::symbol, "nili"sv }
        }));
      }

      SUBCASE("Partial match, suffix")
      {
        processor p{ "onil" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::symbol, "onil"sv }
        }));
      }
      SUBCASE("Comma is whitespace")
      {
        processor p{ "nil," };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::nil }
        }));
      }
      SUBCASE("Semicolon is whitespace")
      {
        processor p{ "nil;" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::nil },
                { 3, 1, token_kind::comment, ""sv }
        }));
      }
    }

    TEST_CASE("Boolean")
    {
      SUBCASE("Full match")
      {
        processor p{ "true false" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::boolean,  true },
                { 5, 5, token_kind::boolean, false }
        }));
      }

      SUBCASE("Partial match, prefix")
      {
        processor p{ "true- falsey" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::symbol,  "true-"sv },
                { 6, 6, token_kind::symbol, "falsey"sv }
        }));
      }

      SUBCASE("Partial match, suffix")
      {
        processor p{ "sotrue ffalse" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 6, token_kind::symbol, "sotrue"sv },
                { 7, 6, token_kind::symbol, "ffalse"sv }
        }));
      }
      SUBCASE("Comma is whitespace")
      {
        processor p{ "true,false," };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::boolean,  true },
                { 5, 5, token_kind::boolean, false }
        }));
      }
      SUBCASE("Semicolon is whitespace")
      {
        processor p{ "true;" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::boolean, true },
                { 4, 1, token_kind::comment, ""sv }
        }));
      }
    }

    TEST_CASE("Ratio")
    {
      SUBCASE("Success - x/x")
      {
        processor p{ "4/5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::ratio, { .numerator = 4, .denominator = 5 } }
        }));
      }
      SUBCASE("Success - -x/x")
      {
        processor p{ "-4/5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::ratio, { .numerator = -4, .denominator = 5 } }
        }));
      }
      SUBCASE("Success - -x/-x")
      {
        processor p{ "-4/-5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::ratio, { .numerator = -4, .denominator = -5 } }
        }));
      }
      SUBCASE("Failures - x//x")
      {
        processor p{ "4//5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ { make_error(kind::lex_invalid_symbol, 2, 2) } }));
      }
      SUBCASE("Failures - x/x/x")
      {
        processor p{ "4/5/4" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { make_error(kind::lex_invalid_ratio, 2, 1) },
                                { make_error(kind::lex_invalid_symbol, 3, 2) } }));
      }
      SUBCASE("Failures - x/x/x/x")
      {
        processor p{ "4/5/4/5/6/7/7" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { make_error(kind::lex_invalid_ratio, 2, 1) },
                                { make_error(kind::lex_invalid_symbol, 3, 10) } }));
      }
      SUBCASE("Failures - x.x/x")
      {
        processor p{ "4.4/5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { make_error(kind::lex_invalid_ratio, 0, 3) },
                                { make_error(kind::lex_invalid_symbol, 3, 3) } }));
      }
      SUBCASE("Failures - x/x.x")
      {
        processor p{ "4/5.9" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ { make_error(kind::lex_invalid_ratio, 0, 5) } }));
      }
      SUBCASE("Failures - xex/x")
      {
        processor p{ "4e1/5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ make_error(kind::lex_invalid_ratio, 0, 3),
                                make_error(kind::lex_invalid_symbol, 3, 3) }));
      }
      SUBCASE("Failures - x/xex")
      {
        processor p{ "4/5e9" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_ratio, 2, 1),
                token{ 3, 2, token_kind::symbol, "e9"sv }
        }));
      }
    }

    TEST_CASE("Integer")
    {
      SUBCASE("Positive single-char")
      {
        processor p{ "0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::integer, 0ll }
        }));
      }

      SUBCASE("Positive multi-char")
      {
        processor p{ "1234" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::integer, 1234ll }
        }));
      }

      SUBCASE("Positive multiple numbers")
      {
        processor p{ "0 1234" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::integer,    0ll },
                { 2, 4, token_kind::integer, 1234ll },
        }));
      }

      SUBCASE("Octal number")
      {
        processor p{ "034 -034 08.9 07e1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 3, token_kind::integer,  28ll },
                {  4, 4, token_kind::integer, -28ll },
                {  9, 4,    token_kind::real,   8.9 },
                { 14, 4,    token_kind::real,  70.0 },
        }));
      }

      SUBCASE("Invalid octal number")
      {
        processor p{ "08 0a -08" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 2),
                make_error(kind::lex_invalid_number, 3, 5),
                make_error(kind::lex_invalid_number, 6, 9),
              }));
      }

      SUBCASE("Octal numbers with invalid padding")
      {
        processor p{ "000123 0123 034 00" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 6, token_kind::integer, 83ll },
                {  7, 4, token_kind::integer, 83ll },
                { 12, 3, token_kind::integer, 28ll },
                { 16, 2, token_kind::integer,  0ll }
        }));
      }

      SUBCASE("Edge case: Invalid mixed radix")
      {
        processor p{ "123abc 0x12g 8r8 16rx.2" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 6),
                make_error(kind::lex_invalid_number, 7, 12),
                make_error(kind::lex_invalid_number, 13, 16),
                make_error(kind::lex_invalid_number, 17, 21),
                make_error(kind::lex_expecting_whitespace, 21, 21),
                token{ 21, 2, token_kind::symbol, ".2"sv },
        }));
      }

      SUBCASE("Arbitrary radix edge cases")
      {
        /* Exceeds 64-bit integer max */
        processor p{ "36r0123456789abcdefghijklmnopqrstuvwxyz" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ make_error(kind::lex_invalid_number, 0, 39) }));

        /* 65 bits. */
        processor p2{ "2r11111111111111111111111111111111111111111111111111111111111111111" };
        native_vector<result<token, error_ptr>> const tokens2(p2.begin(), p2.end());
        CHECK(tokens2 == make_results({ make_error(kind::lex_invalid_number, 0, 67) }));
      }

      SUBCASE("Invalid arbitrary radix edge cases")
      {
        processor p{ "37r1234" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ make_error(kind::lex_invalid_number, 0, 7) }));

        processor p2{ "1r2" };
        native_vector<result<token, error_ptr>> const tokens2(p2.begin(), p2.end());
        CHECK(tokens2 == make_results({ make_error(kind::lex_invalid_number, 0, 3) }));
      }

      SUBCASE("Mixed valid and invalid numbers")
      {
        processor p{ "123 0x1g 8r7 -12 10r1z 16rff" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{  0, 3, token_kind::integer, 123ll },
                make_error(kind::lex_invalid_number, 4, 8),
                token{  9, 3, token_kind::integer,   7ll },
                token{ 13, 3, token_kind::integer, -12ll },
                make_error(kind::lex_invalid_number, 17, 22),
                token{ 23, 5, token_kind::integer, 255ll }
        }));
      }

      SUBCASE("Negative numbers with arbitrary radix")
      {
        processor p{ "-2r11 -8r77 -16rff" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 5, token_kind::integer,   -3ll },
                {  6, 5, token_kind::integer,  -63ll },
                { 12, 6, token_kind::integer, -255ll }
        }));
      }

      SUBCASE("Hex numbers")
      {
        processor p{ "0x34 0Xab 0x12ab 123 0Xef43 -0x1a 0x0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 4, token_kind::integer,    52ll },
                {  5, 4, token_kind::integer,   171ll },
                { 10, 6, token_kind::integer,  4779ll },
                { 17, 3, token_kind::integer,   123ll },
                { 21, 6, token_kind::integer, 61251ll },
                { 28, 5, token_kind::integer,   -26ll },
                { 34, 3, token_kind::integer,     0ll },
        }));
      }

      SUBCASE("Invalid hex numbers")
      {
        processor p{ "0xg 0x-2 0x8.4 0x3e-5 0x 1x" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ make_error(kind::lex_invalid_number, 0, 3),
                                make_error(kind::lex_invalid_number, 4, 8),
                                make_error(kind::lex_invalid_number, 9, 14),
                                make_error(kind::lex_invalid_number, 15, 21),
                                make_error(kind::lex_invalid_number, 22, 24),
                                make_error(kind::lex_invalid_number, 25, 27) }));
      }

      SUBCASE("Invalid hex - 1x1")
      {
        processor p{ "1x1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 3),
              }));
      }

      SUBCASE("Invalid hex - 2.0x1")
      {
        processor p{ "2.0x1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 3,   token_kind::real,    2.0 },
                make_error(kind::lex_invalid_number, 3, 3),
                token{ 3, 2, token_kind::symbol, "x1"sv },
        }));
      }

      SUBCASE("Invalid hex - 0xx1")
      {
        processor p{ "0xx1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 4),
              }));
      }

      SUBCASE("Invalid hex - 0x1x")
      {
        processor p{ "0x1x" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 4),
              }));
      }

      SUBCASE("Invalid hex - 0x.0")
      {
        processor p{ "0x.0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 4),
              }));
      }

      SUBCASE("Valid arbitrary radix")
      {
        processor p{ "2r11 36rz 8R71 19rghi -4r32 16r3e 16r3e4 -32r3e4" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 4, token_kind::integer,     3ll },
                {  5, 4, token_kind::integer,    35ll },
                { 10, 4, token_kind::integer,    57ll },
                { 15, 6, token_kind::integer,  6117ll },
                { 22, 5, token_kind::integer,   -14ll },
                { 28, 5, token_kind::integer,    62ll },
                { 34, 6, token_kind::integer,   996ll },
                { 41, 7, token_kind::integer, -3524ll },
        }));
      }

      SUBCASE("Invalid arbitrary radix")
      {
        processor p{ "2r3 35rz 8re71 19r-ghi 2r 16r" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ make_error(kind::lex_invalid_number, 0, 3),
                                make_error(kind::lex_invalid_number, 4, 8),
                                make_error(kind::lex_invalid_number, 9, 14),
                                make_error(kind::lex_invalid_number, 15, 22),
                                make_error(kind::lex_invalid_number, 23, 25),
                                make_error(kind::lex_invalid_number, 26, 29) }));
      }

      SUBCASE("Invalid arbitrary radix - 0r0")
      {
        processor p{ "0r0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 3),
              }));
      }

      SUBCASE("Invalid arbitrary radix - 1r1")
      {
        processor p{ "1r1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 3),
              }));
      }

      SUBCASE("Invalid arbitrary radix - 2048r0")
      {
        processor p{ "2048r0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 6),
              }));
      }

      SUBCASE("Invalid arbitrary radix - 2.0r1")
      {
        processor p{ "2.0r1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 3),
                token{ 3, 2, token_kind::symbol, "r1"sv },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 2rr1")
      {
        processor p{ "2rr1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 4),
              }));
      }

      SUBCASE("Invalid arbitrary radix - 2r1r")
      {
        processor p{ "2r1r" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 4),
              }));
      }

      SUBCASE("Invalid arbitrary radix - 2r.0")
      {
        processor p{ "2r.0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_number, 0, 2),
                token{ 2, 2, token_kind::symbol, ".0"sv }
        }));
      }

      SUBCASE("Negative single-char")
      {
        processor p{ "-1" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::integer, -1ll }
        }));
      }

      SUBCASE("Negative multi-char")
      {
        processor p{ "-1234" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
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
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 7),
                }));
        }

        SUBCASE("Not required")
        {
          processor p{ "(1234)" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::open_paren },
                  { 1, 4, token_kind::integer, 1234ll },
                  { 5, 1, token_kind::close_paren },
          }));
        }
      }
    }

    TEST_CASE("Real")
    {
      SUBCASE("Positive 0.")
      {
        processor p{ "0." };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::real, 0.0 }
        }));
      }

      SUBCASE("Positive 0.0")
      {
        processor p{ "0.0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::real, 0.0 }
        }));
      }

      SUBCASE("Negative 1.")
      {
        processor p{ "-1." };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::real, -1.0 }
        }));
      }

      SUBCASE("Negative 1.5")
      {
        processor p{ "-1.5" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::real, -1.5 }
        }));
      }

      SUBCASE("Negative multi-char")
      {
        processor p{ "-1234.1234" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 10, token_kind::real, -1234.1234 }
        }));
      }

      SUBCASE("Too many dots")
      {
        {
          processor p{ "0.0." };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 3),
                  token{ 3, 1, token_kind::symbol, "."sv }
          }));
        }
        {
          processor p{ "0..0" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 2),
                  token{ 2, 2, token_kind::symbol, ".0"sv },
          }));
        }
        {
          processor p{ "0.0.0" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 3),
                  token{ 3, 2, token_kind::symbol, ".0"sv },
          }));
        }
      }

      SUBCASE("Expect space")
      {
        SUBCASE("Required")
        {
          processor p{ "12.34abc" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 5,   token_kind::real,   12.34 },
                  make_error(kind::lex_expecting_whitespace, 5, 0),
                  token{ 5, 3, token_kind::symbol, "abc"sv },
          }));
        }

        SUBCASE("Not required")
        {
          processor p{ "(12.34)" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::open_paren },
                  { 1, 5, token_kind::real, 12.34 },
                  { 6, 1, token_kind::close_paren },
          }));
        }
      }

      SUBCASE("Scientific notation")
      {
        SUBCASE("Valid")
        {
          processor p{ "1e3 -1e2 2.E-3 22.3e-8 -12E+18\\a" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  {  0, 3,      token_kind::real,   1000.0 },
                  {  4, 4,      token_kind::real,   -100.0 },
                  {  9, 5,      token_kind::real,    0.002 },
                  { 15, 7,      token_kind::real, 2.23e-07 },
                  { 23, 7,      token_kind::real, -1.2e+19 },
                  { 30, 2, token_kind::character,  "\\a"sv },
          }));
        }

        SUBCASE("Missing exponent")
        {
          processor p{ "1e 23E-1 12e- -0.2e" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 2),
                  token{ 3, 5, token_kind::real, 2.3 },
                  make_error(kind::lex_invalid_number, 9, 13),
                  make_error(kind::lex_invalid_number, 14, 19),
          }));
        }

        SUBCASE("Signs after exponent found")
        {
          processor p{ "12.3 -1e3- 2.3E+" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 4,   token_kind::real,  12.3 },
                  make_error(kind::lex_invalid_number, 5, 9),
                  make_error(kind::lex_expecting_whitespace, 9, 0),
                  token{ 9, 1, token_kind::symbol, "-"sv },
                  make_error(kind::lex_invalid_number, 11, 16),
          }));
        }

        SUBCASE("Extra dots")
        {
          processor p{ "1e3. 12.3 -1e4.3" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 3),
                  token{  3, 1, token_kind::symbol,  "."sv },
                  token{  5, 4,   token_kind::real,   12.3 },
                  make_error(kind::lex_invalid_number, 10, 14),
                  make_error(kind::lex_expecting_whitespace, 14, 0),
                  token{ 14, 2, token_kind::symbol, ".3"sv },
          }));
        }

        SUBCASE("Extra characters in exponent")
        {
          processor p{ "2.ee4 -1e4E3 1.eFoo 3E5fOo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_number, 0, 3),
                  token{  3, 2, token_kind::symbol,  "e4"sv },
                  make_error(kind::lex_invalid_number, 6, 10),
                  make_error(kind::lex_expecting_whitespace, 10, 0),
                  token{ 10, 2, token_kind::symbol,  "E3"sv },
                  make_error(kind::lex_invalid_number, 13, 16),
                  make_error(kind::lex_expecting_whitespace, 16, 0),
                  token{ 16, 3, token_kind::symbol, "Foo"sv },
                  make_error(kind::lex_invalid_number, 20, 26),
          }));
        }
      }
    }

    TEST_CASE("Character")
    {
      SUBCASE("Whitespace after \\")
      {
        processor p{ R"(\ )" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ { make_error(kind::lex_incomplete_character, 0, 1) } }));
      }

      SUBCASE("Dangling \\")
      {
        processor p{ R"(\)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ { make_error(kind::lex_unexpected_eof, 0, 0) } }));
      }

      SUBCASE("Alphabetic")
      {
        processor p{ R"(\a)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::character, "\\a"sv }
        }));
      }

      SUBCASE("Numeric")
      {
        processor p{ R"(\1)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::character, "\\1"sv }
        }));
      }

      SUBCASE("Multiple characters after \\")
      {
        processor p{ R"(\11)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::character, "\\11"sv }
        }));
      }

      SUBCASE("Valid lexed character with symbol")
      {
        processor p{ R"(\1:)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::character, "\\1:"sv },
        }));
      }

      SUBCASE("Valid consecutive characters")
      {
        processor p{ R"(\1 \newline\' \\)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 2, token_kind::character,       "\\1"sv },
                {  3, 8, token_kind::character, "\\newline"sv },
                { 11, 2, token_kind::character,       "\\'"sv },
                { 14, 2, token_kind::character,      "\\\\"sv }
        }));
      }

      SUBCASE("Character followed by a backticked keyword")
      {
        processor p{ R"(\a`:kw)" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::character, "\\a"sv },
                { 2, 1, token_kind::syntax_quote },
                { 3, 3, token_kind::keyword, "kw"sv }
        }));
      }
    }

    TEST_CASE("Symbol")
    {
      SUBCASE("Single-char")
      {
        processor p{ "a" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::symbol, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ "abc" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::symbol, "abc"sv }
        }));
      }

      SUBCASE("Single slash")
      {
        processor p{ "/" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::symbol, "/"sv }
        }));
      }

      SUBCASE("Multi slash")
      {
        processor p{ "//" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_symbol, 0, 2),
              }));
      }

      SUBCASE("Starting with a slash")
      {
        processor p{ "/foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_symbol, 0, 4),
              }));
      }

      SUBCASE("With numbers")
      {
        processor p{ "abc123" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 6, token_kind::symbol, "abc123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ "abc_.123/-foo+?=!&<>#%" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 22, token_kind::symbol, "abc_.123/-foo+?=!&<>#%"sv }
        }));
      }

      SUBCASE("Only -")
      {
        processor p{ "-" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::symbol, "-"sv }
        }));
      }

      SUBCASE("Starting with -")
      {
        processor p{ "-main" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::symbol, "-main"sv }
        }));
      }

      SUBCASE("Quoted")
      {
        processor p{ "'foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::single_quote },
                { 1, 3, token_kind::symbol, "foo"sv }
        }));
      }

      SUBCASE("Starting with .")
      {
        processor p{ ".foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::symbol, ".foo"sv }
        }));
      }

      SUBCASE("Positive no leading digit")
      {
        processor p{ ".0" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 2, token_kind::symbol, ".0"sv },
        }));
      }

      //TODO: https://github.com/jank-lang/jank/issues/223
      //SUBCASE("Negative no leading digit")
      //{
      //  processor p{ "-.0" };
      //  native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
      //  CHECK(tokens
      //        == make_results({
      //          token{ 0, 3, token_kind::symbol, "-.0"sv },
      //  }));
      //}
    }

    TEST_CASE("Keyword")
    {
      SUBCASE("Single-char")
      {
        processor p{ ":a" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::keyword, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ ":abc" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::keyword, "abc"sv }
        }));
      }

      SUBCASE("Whitespace after :")
      {
        processor p{ ": " };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ make_error(kind::lex_invalid_keyword, 0, 1) }));
      }

      SUBCASE("Comma after :")
      {
        processor p{ ":," };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ make_error(kind::lex_invalid_keyword, 0, 1) }));
      }

      SUBCASE("Single slash")
      {
        processor p{ ":/" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::keyword, "/"sv }
        }));
      }

      SUBCASE("Multi slash")
      {
        processor p{ "://" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_keyword, 0, 3),
              }));
      }

      SUBCASE("Starting with a slash")
      {
        processor p{ ":/foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_keyword, 0, 5),
              }));
      }

      SUBCASE("With numbers")
      {
        processor p{ ":abc123" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 7, token_kind::keyword, "abc123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ ":abc_.123/-foo+?=!&<>" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 21, token_kind::keyword, "abc_.123/-foo+?=!&<>"sv }
        }));
      }

      SUBCASE("Auto-resolved unqualified")
      {
        processor p{ "::foo-bar" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 9, token_kind::keyword, ":foo-bar"sv }
        }));
      }

      SUBCASE("Auto-resolved qualified")
      {
        processor p{ "::foo/bar" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 9, token_kind::keyword, ":foo/bar"sv }
        }));
      }

      SUBCASE("Too many starting colons")
      {
        processor p{ ":::foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_keyword, 0, 6),
              }));
      }

      SUBCASE("Way too many starting colons")
      {
        processor p{ "::::foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_keyword, 0, 7),
              }));
      }
    }

    TEST_CASE("String")
    {
      SUBCASE("Empty")
      {
        processor p{ "\"\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 2, token_kind::string, ""sv }
        }));
      }
      SUBCASE("Single-char")
      {
        processor p{ "\"a\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::string, "a"sv }
        }));
      }
      SUBCASE("Question mark")
      {
        processor p{ "\"?\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 3, token_kind::string, "?"sv }
        }));
      }
      SUBCASE("Escaped Question mark")
      {
        processor p{ R"("\?")" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 4, token_kind::escaped_string, "\\?"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ "\"abc\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::string, "abc"sv }
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ "\"123\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 5, token_kind::string, "123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ "\"and then() there was abc_123/-foo?!&<>\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 40, token_kind::string, "and then() there was abc_123/-foo?!&<>"sv }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "\"foo\nbar\nspam\t\"" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { { 0, 1, 1 }, { 15, 3, 7 }, token_kind::string, "foo\nbar\nspam\t"sv }
        }));
      }

      SUBCASE("With escapes")
      {
        processor p{ R"("foo\"\nbar\nspam\t\r")" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 22, token_kind::escaped_string, "foo\\\"\\nbar\\nspam\\t\\r"sv }
        }));

        processor q{ R"("\??\' \\ a\a b\b f\f v\v")" };
        native_vector<result<token, error_ptr>> const tokens2(q.begin(), q.end());
        CHECK(tokens2
              == make_tokens({
                { 0, 26, token_kind::escaped_string, "\\\?\?\\' \\\\ a\\a b\\b f\\f v\\v"sv }
        }));
      }

      SUBCASE("Unicode characters")
      {
        processor p{ R"("ሴ 你好")" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 12, token_kind::string, "ሴ 你好"sv }
        }));
      }

      SUBCASE("Unterminated")
      {
        processor p{ "\"meow" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_unterminated_string, 0, 5),
              }));
      }
    }

    TEST_CASE("Meta hint")
    {
      SUBCASE("Empty")
      {
        processor p{ "^" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::meta_hint }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "^\n:foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { { 0, 1, 1 }, { 1, 1, 2 }, token_kind::meta_hint },
                { { 2, 2, 1 }, { 6, 2, 5 }, token_kind::keyword, "foo"sv }
        }));
      }
    }

    TEST_CASE("Reader macro")
    {
      SUBCASE("Empty")
      {
        processor p{ "#" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
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
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_comment }
          }));
        }

        SUBCASE("No whitespace after")
        {
          processor p{ "#_[]" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_comment },
                  { 2, 1,  token_kind::open_square_bracket },
                  { 3, 1, token_kind::close_square_bracket }
          }));
        }
      }

      SUBCASE("Shebang Comments")
      {
        SUBCASE("Empty")
        {
          processor p{ "#!" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::comment, ""sv }
          }));
        }

        SUBCASE("Empty multi-line")
        {
          processor p{ "#!\n#!" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  {           0,           2, token_kind::comment, ""sv },
                  { { 3, 2, 1 }, { 5, 2, 3 }, token_kind::comment, ""sv }
          }));
        }

        SUBCASE("Non-empty")
        {
          processor p{ "#!foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 5, token_kind::comment, "foo"sv },
          }));
        }

        SUBCASE("Multiple on same line")
        {
          processor p{ "#!foo #!bar" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 11, token_kind::comment, "foo #!bar"sv }
          }));
        }

        SUBCASE("Multiple on same line; last is empty")
        {
          processor p{ "#!foo #!" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 8, token_kind::comment, "foo #!"sv }
          }));
        }

        SUBCASE("Multiple #! in a row")
        {
          processor p{ "#!#!#!foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 9, token_kind::comment, "#!#!foo"sv }
          }));
        }

        SUBCASE("Expressions before")
        {
          processor p{ "1 2 #!foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::integer,     1ll },
                  { 2, 1, token_kind::integer,     2ll },
                  { 4, 5, token_kind::comment, "foo"sv }
          }));
        }

        SUBCASE("Expressions before and after")
        {
          processor p{ "1 #!foo\n2" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  {           0,           1, token_kind::integer,     1ll },
                  {           2,           5, token_kind::comment, "foo"sv },
                  { { 8, 2, 1 }, { 9, 2, 2 }, token_kind::integer,     2ll }
          }));
        }

        SUBCASE("Multiple lines starting with #!")
        {
          processor p{ "#!foo\n#!bar" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  {           0,            5, token_kind::comment, "foo"sv },
                  { { 6, 2, 1 }, { 11, 2, 6 }, token_kind::comment, "bar"sv },
          }));
        }

        SUBCASE("Double #")
        {
          processor p{ "##!foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::reader_macro },
                  { 1, 5, token_kind::comment, "foo"sv },
          }));
        }

        SUBCASE("Double !")
        {
          processor p{ "#!!foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 6, token_kind::comment, "!foo"sv },
          }));
        }

        SUBCASE("Hash Bang Hash")
        {
          processor p{ "#!#foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 6, token_kind::comment, "#foo"sv },
          }));
        }

        SUBCASE("Don't parse list")
        {
          processor p{ "#!(+ 1 1)" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 9, token_kind::comment, "(+ 1 1)"sv },
          }));
        }

        SUBCASE("Don't parse string")
        {
          processor p{ "#!\"foo\"" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 7, token_kind::comment, "\"foo\""sv },
          }));
        }

        SUBCASE("Shebang in list")
        {
          processor p{ "(#!)" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::open_paren },
                  { 1, 3, token_kind::comment, ")"sv },
          }));
        }
      }

      SUBCASE("Conditional")
      {
        SUBCASE("Empty")
        {
          processor p{ "#?" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::reader_macro_conditional }
          }));
        }

        SUBCASE("With following list")
        {
          processor p{ "#?()" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
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
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
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
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  {           0,           1,        token_kind::reader_macro },
                  { { 2, 2, 1 }, { 3, 2, 2 },  token_kind::open_curly_bracket },
                  { { 3, 2, 2 }, { 4, 2, 3 }, token_kind::close_curly_bracket }
          }));
        }
      }
    }

    TEST_CASE("Syntax quoting")
    {
      SUBCASE("Empty")
      {
        processor p{ "`" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::syntax_quote }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "`\n:foo" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, token_kind::syntax_quote },
                { { 2, 2, 1 }, { 6, 2, 5 }, token_kind::keyword, "foo"sv }
        }));
      }

      SUBCASE("Unquote")
      {
        SUBCASE("Empty")
        {
          processor p{ "~" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::unquote }
          }));
        }

        SUBCASE("With line breaks")
        {
          processor p{ "~\n:foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::unquote },
                  { { 2, 2, 1 }, { 6, 2, 5 }, token_kind::keyword, "foo"sv }
          }));
        }
      }

      SUBCASE("Unquote splicing")
      {
        SUBCASE("Empty")
        {
          processor p{ "~@" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::unquote_splice }
          }));
        }

        SUBCASE("With line breaks before")
        {
          processor p{ "~\n@:foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, token_kind::unquote },
                  { { 2, 2, 1 }, { 3, 2, 2 }, token_kind::deref },
                  { { 3, 2, 2 }, { 7, 2, 6 }, token_kind::keyword, "foo"sv }
          }));
        }

        SUBCASE("With line breaks after")
        {
          processor p{ "~@\n:foo" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 2, token_kind::unquote_splice },
                  { { 3, 2, 1 }, { 7, 2, 5 }, token_kind::keyword, "foo"sv }
          }));
        }
      }
    }

    TEST_CASE("UTF-8")
    {
      SUBCASE("Symbols")
      {
        SUBCASE("Single character")
        {
          processor p{ "👍" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 4, token_kind::symbol, "👍"sv }
          }));
        }
        SUBCASE("Multiple characters")
        {
          processor p{ "😎👍" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 8, token_kind::symbol, "😎👍"sv }
          }));
        }
        SUBCASE("Symbol with mixed characters inside")
        {
          processor p{ "one-🍺-please!" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 16, token_kind::symbol, "one-🍺-please!"sv }
          }));
        }
        SUBCASE("Qualified Symbol")
        {
          processor p{ "🐝/🥀" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 9, token_kind::symbol, "🐝/🥀"sv }
          }));
        }
        SUBCASE("Comma is whitespace")
        {
          processor p{ "abc," };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 3, token_kind::symbol, "abc"sv }
          }));
        }
      }
      SUBCASE("Keywords")
      {
        SUBCASE("Single character")
        {
          processor p{ ":🍙" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 5, token_kind::keyword, "🍙"sv },
          }));
        }
        SUBCASE("Multiple characters keyword")
        {
          processor p{ ":🥩🍗" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 9, token_kind::keyword, "🥩🍗"sv }
          }));
        }
        SUBCASE("Keyword with mixed characters inside")
        {
          processor p{ ":w🍪w" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 7, token_kind::keyword, "w🍪w"sv }
          }));
        }
        SUBCASE("Qualified Keyword")
        {
          processor p{ ":🐝/🥀" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 10, token_kind::keyword, "🐝/🥀"sv }
          }));
        }
        SUBCASE("Single comma is an empty keyword")
        {
          processor p{ ":," };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens == make_results({ make_error(kind::lex_invalid_keyword, 0, 1) }));
        }
        SUBCASE("Single semicolon is an empty keyword")
        {
          processor p{ ":;" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_keyword, 0, 1),
                  token{ 1, 1, token_kind::comment, ""sv }
          }));
        }
        SUBCASE("Single paren makes an empty keyword")
        {
          processor p{ ":)" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  make_error(kind::lex_invalid_keyword, 0, 1),
                  token{ 1, 1, token_kind::close_paren }
          }));
        }
        SUBCASE("Comma is whitespace")
        {
          processor p{ ":abc," };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 4, token_kind::keyword, "abc"sv }
          }));
        }
        SUBCASE("Semicolon is whitespace")
        {
          processor p{ ":abc;" };
          native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 4, token_kind::keyword, "abc"sv },
                  { 4, 1, token_kind::comment,    ""sv }
          }));
        }
      }
      SUBCASE("8-wide character")
      {
        processor p{ "🇪🇺" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 8, token_kind::symbol, "🇪🇺"sv }
        }));
      }
      SUBCASE("Non-Latin Characters")
      {
        processor p{ "ありがとう" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 15, token_kind::symbol, "ありがとう"sv }
        }));
      }
      SUBCASE("Non-Latin Characters")
      {
        processor p{ ":ありがとう" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 16, token_kind::keyword, "ありがとう"sv }
        }));
      }
      SUBCASE("Whitespace Characters")
      {
        processor p{ ":  " };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 7, token_kind::keyword, "  "sv }
        }));
      }


      SUBCASE("Malformed Text")
      {
        processor p{ "\xC0\x80" };
        native_vector<result<token, error_ptr>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                make_error(kind::lex_invalid_unicode, 0, 0),
                make_error(kind::lex_invalid_unicode, 1, 0),
              }));
      }
    }
  }
}
