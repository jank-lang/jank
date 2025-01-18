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
  static constexpr std::array<result<token, error>, N>
  make_results(result<token, error> const (&arr)[N])
  {
    return detail::to_array(arr);
  }

  template <size_t N>
  static bool
  operator==(native_vector<result<token, error>> const &v, std::array<token, N> const &a)
  {
    if(v.size() != N)
    {
      return false;
    }
    return std::equal(v.begin(), v.end(), a.begin());
  }

  template <size_t N>
  static bool operator==(native_vector<result<token, error>> const &v,
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
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::comment, ""sv }
        }));
      }

      SUBCASE("Empty multi-line")
      {
        processor p{ ";\n;" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::comment, ""sv },
                { 2, 2, 1, 1, token_kind::comment, ""sv }
        }));
      }

      SUBCASE("Non-empty")
      {
        processor p{ "; Hello hello" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 12, token_kind::comment, " Hello hello"sv }
        }));
      }

      SUBCASE("Multiple on same line")
      {
        processor p{ "; Hello hello ; \"hi hi\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 22, token_kind::comment, " Hello hello ; \"hi hi\""sv }
        }));
      }

      SUBCASE("Multiple ; in a row")
      {
        processor p{ ";;; Hello hello 12" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 17, token_kind::comment, " Hello hello 12"sv }
        }));
      }

      SUBCASE("Expressions before")
      {
        processor p{ "1 2 ; meow" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::integer,       1ll },
                { 2, 1, 1, 1, token_kind::integer,       2ll },
                { 4, 1, 1, 5, token_kind::comment, " meow"sv }
        }));
      }

      SUBCASE("Expressions before and after")
      {
        processor p{ "1 ; meow\n2" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::integer,       1ll },
                { 2, 1, 1, 5, token_kind::comment, " meow"sv },
                { 9, 1, 1, 1, token_kind::integer,       2ll }
        }));
      }
    }

    TEST_CASE("List")
    {
      SUBCASE("Empty")
      {
        processor p{ "()" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1,  token_kind::open_paren },
                { 1, 1, 1, token_kind::close_paren }
        }));
      }

      SUBCASE("Nested")
      {
        processor p{ "(())" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1,  token_kind::open_paren },
                { 1, 1, 1,  token_kind::open_paren },
                { 2, 1, 1, token_kind::close_paren },
                { 3, 1, 1, token_kind::close_paren }
        }));
      }

      SUBCASE("Unbalanced")
      {
        SUBCASE("Open")
        {
          processor p{ "(" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, token_kind::open_paren }
          }));
        }

        SUBCASE("Closed")
        {
          processor p{ ")" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, token_kind::close_paren }
          }));
        }
      }
    }

    TEST_CASE("Vector")
    {
      SUBCASE("Empty")
      {
        processor p{ "[]" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1,  token_kind::open_square_bracket },
                { 1, 1, 1, token_kind::close_square_bracket }
        }));
      }

      SUBCASE("Nested")
      {
        processor p{ "[[]]" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1,  token_kind::open_square_bracket },
                { 1, 1, 1,  token_kind::open_square_bracket },
                { 2, 1, 1, token_kind::close_square_bracket },
                { 3, 1, 1, token_kind::close_square_bracket }
        }));
      }

      SUBCASE("Mixed")
      {
        processor p{ "[(foo [1 2]) 2]" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, token_kind::open_square_bracket },
                { 1, 1, 1, token_kind::open_paren },
                { 2, 1, 1, 3, token_kind::symbol, "foo"sv },
                { 6, 1, 1, token_kind::open_square_bracket },
                { 7, 1, 1, token_kind::integer, 1ll },
                { 9, 1, 1, token_kind::integer, 2ll },
                { 10, 1, 1, token_kind::close_square_bracket },
                { 11, 1, 1, token_kind::close_paren },
                { 13, 1, 1, token_kind::integer, 2ll },
                { 14, 1, 1, token_kind::close_square_bracket },
        }));
      }

      SUBCASE("Unbalanced")
      {
        SUBCASE("Open")
        {
          processor p{ "[" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, token_kind::open_square_bracket }
          }));
        }

        SUBCASE("Closed")
        {
          processor p{ "]" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, token_kind::close_square_bracket }
          }));
        }
      }
    }

    TEST_CASE("Nil")
    {
      SUBCASE("Full match")
      {
        processor p{ "nil" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::nil }
        }));
      }

      SUBCASE("Partial match, prefix")
      {
        processor p{ "nili" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::symbol, "nili"sv }
        }));
      }

      SUBCASE("Partial match, suffix")
      {
        processor p{ "onil" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::symbol, "onil"sv }
        }));
      }
    }

    TEST_CASE("Boolean")
    {
      SUBCASE("Full match")
      {
        processor p{ "true false" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::boolean,  true },
                { 5, 1, 1, 5, token_kind::boolean, false }
        }));
      }

      SUBCASE("Partial match, prefix")
      {
        processor p{ "true- falsey" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 5, token_kind::symbol,  "true-"sv },
                { 6, 1, 1, 6, token_kind::symbol, "falsey"sv }
        }));
      }

      SUBCASE("Partial match, suffix")
      {
        processor p{ "sotrue ffalse" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 6, token_kind::symbol, "sotrue"sv },
                { 7, 1, 1, 6, token_kind::symbol, "ffalse"sv }
        }));
      }
    }
    TEST_CASE("Ratio")
    {
      SUBCASE("Success - x/x")
      {
        processor p{ "4/5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::ratio, { .numerator = 4, .denominator = 5 } }
        }));
      }
      SUBCASE("Success - -x/x")
      {
        processor p{ "-4/5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::ratio, { .numerator = -4, .denominator = 5 } }
        }));
      }
      SUBCASE("Success - -x/-x")
      {
        processor p{ "-4/-5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 5, token_kind::ratio, { .numerator = -4, .denominator = -5 } }
        }));
      }
      SUBCASE("Failures - x//x")
      {
        processor p{ "4//5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens == make_results({ { error(2, 2, "invalid symbol") } }));
      }
      SUBCASE("Failures - x/x/x")
      {
        processor p{ "4/5/4" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(
          tokens
          == make_results({ { error(2, 3, "invalid ratio") }, { error(3, 3, "invalid symbol") } }));
      }
      SUBCASE("Failures - x/x/x/x")
      {
        processor p{ "4/5/4/5/6/7/7" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(
          tokens
          == make_results({ { error(2, 3, "invalid ratio") }, { error(3, 3, "invalid symbol") } }));
      }
      SUBCASE("Failures - x.x/x")
      {
        processor p{ "4.4/5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(
          tokens
          == make_results({ { error(0, 3, "invalid ratio") }, { error(3, 3, "invalid symbol") } }));
      }
      SUBCASE("Failures - x/x.x")
      {
        processor p{ "4/5.9" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(
          tokens
          == make_results({ { error(0, 5, "invalid ratio: expecting an integer denominator") } }));
      }
      SUBCASE("Failures - xex/x")
      {
        processor p{ "4e1/5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 3,  "invalid ratio" },
                error{ 3, 3, "invalid symbol" }
        }));
      }
      SUBCASE("Failures - x/xex")
      {
        processor p{ "4/5e9" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 2, 3, "invalid ratio: ratio cannot have scientific notation" },
                token{ 3, 1, 1, 2, token_kind::symbol, "e9"sv }
        }));
      }
    }
    TEST_CASE("Integer")
    {
      SUBCASE("Positive single-char")
      {
        processor p{ "0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, token_kind::integer, 0ll }
        }));
      }

      SUBCASE("Positive multi-char")
      {
        processor p{ "1234" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::integer, 1234ll }
        }));
      }

      SUBCASE("Positive multiple numbers")
      {
        processor p{ "0 1234" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::integer,    0ll },
                { 2, 1, 1, 4, token_kind::integer, 1234ll },
        }));
      }

      SUBCASE("Octal number")
      {
        processor p{ "034 -034 08.9 07e1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 1, 1, 3, token_kind::integer,  28ll },
                {  4, 1, 1, 4, token_kind::integer, -28ll },
                {  9, 1, 1, 4,    token_kind::real,   8.9 },
                { 14, 1, 1, 4,    token_kind::real,  70.0 },
        }));
      }

      SUBCASE("Invalid octal number")
      {
        processor p{ "08 0a -08" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 2, "invalid number: chars '8' are invalid for radix 8" },
                error{ 3, 5, "invalid number: chars 'a' are invalid for radix 8" },
                error{ 6, 9, "invalid number: chars '8' are invalid for radix 8" },
        }));
      }

      SUBCASE("Octal numbers with invalid padding")
      {
        processor p{ "000123 0123 034 00" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 1, 1, 6, token_kind::integer, 83ll },
                {  7, 1, 1, 4, token_kind::integer, 83ll },
                { 12, 1, 1, 3, token_kind::integer, 28ll },
                { 16, 1, 1, 2, token_kind::integer,  0ll }
        }));
      }

      SUBCASE("Edge case: Invalid mixed radix")
      {
        processor p{ "123abc 0x12g 8r8 16rx.2" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 6, "invalid number: chars 'abc' are invalid for radix 10" },
                error{ 7, 12, "invalid number: chars 'g' are invalid for radix 16" },
                error{ 13, 16, "invalid number: chars '8' are invalid for radix 8" },
                error{ 17, 21, "arbitrary radix number can only be an integer" },
                error{ 21, 21, "unexpected character: ." },
                error{ 22, 22, "expected whitespace before next token" },
                token{ 22, 1, 1, 1, token_kind::integer, 2ll },
        }));
      }

      SUBCASE("Arbitrary radix edge cases")
      {
        /* exceeds 64-bit integer max */
        processor p{ "36r0123456789abcdefghijklmnopqrstuvwxyz" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 39, "number out of range" }
        }));

        processor p2{ "2r1111111111111111111111111111111111111111111" };
        native_vector<result<token, error>> const tokens2(p2.begin(), p2.end());
        CHECK(tokens2
              == make_tokens({
                { 0, 1, 1, 45, token_kind::integer, 8796093022207ll }
        }));
      }

      SUBCASE("Invalid arbitrary radix edge cases")
      {
        processor p{ "37r1234" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 7, "invalid number: radix 37 is out of range" }
        }));

        processor p2{ "1r2" };
        native_vector<result<token, error>> const tokens2(p2.begin(), p2.end());
        CHECK(tokens2
              == make_results({
                error{ 0, 3, "invalid number: radix 1 is out of range" }
        }));
      }

      SUBCASE("Mixed valid and invalid numbers")
      {
        processor p{ "123 0x1g 8r7 -12 10r1z 16rff" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 1, 1, 3, token_kind::integer, 123ll },
                error{ 4, 8, "invalid number: chars 'g' are invalid for radix 16" },
                token{ 9, 1, 1, 3, token_kind::integer, 7ll },
                token{ 13, 1, 1, 3, token_kind::integer, -12ll },
                error{ 17, 22, "invalid number: chars 'z' are invalid for radix 10" },
                token{ 23, 1, 1, 5, token_kind::integer, 255ll }
        }));
      }

      SUBCASE("Negative numbers with arbitrary radix")
      {
        processor p{ "-2r11 -8r77 -16rff" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 1, 1, 5, token_kind::integer,   -3ll },
                {  6, 1, 1, 5, token_kind::integer,  -63ll },
                { 12, 1, 1, 6, token_kind::integer, -255ll }
        }));
      }

      SUBCASE("Hex numbers")
      {
        processor p{ "0x34 0Xab 0x12ab 123 0Xef43 -0x1a 0x0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 1, 1, 4, token_kind::integer,    52ll },
                {  5, 1, 1, 4, token_kind::integer,   171ll },
                { 10, 1, 1, 6, token_kind::integer,  4779ll },
                { 17, 1, 1, 3, token_kind::integer,   123ll },
                { 21, 1, 1, 6, token_kind::integer, 61251ll },
                { 28, 1, 1, 5, token_kind::integer,   -26ll },
                { 34, 1, 1, 3, token_kind::integer,     0ll },
        }));
      }

      SUBCASE("Invalid hex numbers")
      {
        processor p{ "0xg 0x-2 0x8.4 0x3e-5 0x 1x" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{  0,  3,       "invalid number: chars 'g' are invalid for radix 16" },
                error{  4,  8,       "invalid number: chars '-' are invalid for radix 16" },
                error{  9, 14,       "invalid number: chars '.' are invalid for radix 16" },
                error{ 15, 21,       "invalid number: chars '-' are invalid for radix 16" },
                error{ 22, 24, "unexpected end of radix 16 number, expecting more digits" },
                error{ 25, 27,       "invalid number: chars 'x' are invalid for radix 10" }
        }));
      }

      SUBCASE("Invalid hex - 1x1")
      {
        processor p{ "1x1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 3, "invalid number: chars 'x' are invalid for radix 10" },
        }));
      }

      SUBCASE("Invalid hex - 2.0x1")
      {
        processor p{ "2.0x1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 1, 1, 3, token_kind::real, 2.0 },
                error{ 3, 3, "expected whitespace before next token" },
                token{ 3, 1, 1, 2, token_kind::symbol, "x1"sv },
        }));
      }

      SUBCASE("Invalid hex - 0xx1")
      {
        processor p{ "0xx1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 4, "invalid number: chars 'x' are invalid for radix 16" },
        }));
      }

      SUBCASE("Invalid hex - 0x1x")
      {
        processor p{ "0x1x" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 4, "invalid number: chars 'x' are invalid for radix 16" },
        }));
      }

      SUBCASE("Invalid hex - 0x.0")
      {
        processor p{ "0x.0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 4, "invalid number: chars '.' are invalid for radix 16" },
        }));
      }

      SUBCASE("Valid arbitrary radix")
      {
        processor p{ "2r11 36rz 8R71 19rghi -4r32 16r3e 16r3e4 -32r3e4" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{  0, 1, 1, 4, token_kind::integer,     3ll },
                token{  5, 1, 1, 4, token_kind::integer,    35ll },
                token{ 10, 1, 1, 4, token_kind::integer,    57ll },
                token{ 15, 1, 1, 6, token_kind::integer,  6117ll },
                token{ 22, 1, 1, 5, token_kind::integer,   -14ll },
                token{ 28, 1, 1, 5, token_kind::integer,    62ll },
                token{ 34, 1, 1, 6, token_kind::integer,   996ll },
                token{ 41, 1, 1, 7, token_kind::integer, -3524ll },
        }));
      }

      SUBCASE("Invalid arbitrary radix")
      {
        processor p{ "2r3 35rz 8re71 19r-ghi 2r 16r" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{  0,  3,     "invalid number: chars '3' are invalid for radix 2" },
                error{  4,  8,    "invalid number: chars 'z' are invalid for radix 35" },
                error{  9, 14,     "invalid number: chars 'e' are invalid for radix 8" },
                error{ 15, 22,    "invalid number: chars '-' are invalid for radix 19" },
                error{ 23, 25, "unexpected end of radix number, expecting more digits" },
                error{ 26, 29, "unexpected end of radix number, expecting more digits" }
        }));
      }

      SUBCASE("Invalid arbitrary radix - 0r0")
      {
        processor p{ "0r0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 3, "invalid number: radix 0 is out of range" },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 1r1")
      {
        processor p{ "1r1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 3, "invalid number: radix 1 is out of range" },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 2048r0")
      {
        processor p{ "2048r0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 6, "invalid number: radix 2048 is out of range" },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 2.0r1")
      {
        processor p{ "2.0r1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 3, "invalid number: arbitrary radix numbers can only be integers" },
                token{ 3, 1, 1, 2, token_kind::symbol, "r1"sv },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 2rr1")
      {
        processor p{ "2rr1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 4, "invalid number: chars 'r' are invalid for radix 2" },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 2r1r")
      {
        processor p{ "2r1r" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 4, "invalid number: chars 'r' are invalid for radix 2" },
        }));
      }

      SUBCASE("Invalid arbitrary radix - 2r.0")
      {
        processor p{ "2r.0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 2, "arbitrary radix number can only be an integer" },
                error{ 2, 2, "unexpected character: ." },
                token{ 3, 1, 1, 1, token_kind::integer, 0ll }
        }));
      }

      SUBCASE("Negative single-char")
      {
        processor p{ "-1" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::integer, -1ll }
        }));
      }

      SUBCASE("Negative multi-char")
      {
        processor p{ "-1234" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 5, token_kind::integer, -1234ll }
        }));
      }

      SUBCASE("Expect space")
      {
        SUBCASE("Required")
        {
          processor p{ "1234abc" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 7, "invalid number: chars 'abc' are invalid for radix 10" },
          }));
        }

        SUBCASE("Not required")
        {
          processor p{ "(1234)" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 1, 1, token_kind::open_paren },
                  token{ 1, 1, 1, 4, token_kind::integer, 1234ll },
                  token{ 5, 1, 1, token_kind::close_paren },
          }));
        }
      }
    }

    TEST_CASE("Real")
    {
      SUBCASE("Positive 0.")
      {
        processor p{ "0." };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::real, 0.0 }
        }));
      }

      SUBCASE("Positive 0.0")
      {
        processor p{ "0.0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::real, 0.0 }
        }));
      }

      SUBCASE("Negative 1.")
      {
        processor p{ "-1." };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::real, -1.0 }
        }));
      }

      SUBCASE("Negative 1.5")
      {
        processor p{ "-1.5" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::real, -1.5 }
        }));
      }

      SUBCASE("Negative multi-char")
      {
        processor p{ "-1234.1234" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 10, token_kind::real, -1234.1234 }
        }));
      }

      SUBCASE("Positive no leading digit")
      {
        processor p{ ".0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "unexpected character: ." },
                token{ 1, 1, 1, token_kind::integer, 0ll },
        }));
      }

      SUBCASE("Negative no leading digit")
      {
        processor p{ "-.0" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, 1, "invalid number" },
                error{ 1, "unexpected character: ." },
                token{ 2, 1, 1, token_kind::integer, 0ll },
        }));
      }

      SUBCASE("Too many dots")
      {
        {
          processor p{ "0.0." };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 3, "invalid number" },
                  error{ 3, "unexpected character: ." },
          }));
        }
        {
          processor p{ "0..0" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 2, "invalid number" },
                  error{ 2, "unexpected character: ." },
                  token{ 3, 1, 1, token_kind::integer, 0ll },
          }));
        }
        {
          processor p{ "0.0.0" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 3, "invalid number" },
                  error{ 3, "unexpected character: ." },
                  token{ 4, 1, 1, token_kind::integer, 0ll },
          }));
        }
      }

      SUBCASE("Expect space")
      {
        SUBCASE("Required")
        {
          processor p{ "12.34abc" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 1, 1, 5, token_kind::real, 12.34 },
                  error{ 5, "expected whitespace before next token" },
                  token{ 5, 1, 1, 3, token_kind::symbol, "abc"sv },
          }));
        }

        SUBCASE("Not required")
        {
          processor p{ "(12.34)" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 1, 1, token_kind::open_paren },
                  token{ 1, 1, 1, 5, token_kind::real, 12.34 },
                  token{ 6, 1, 1, token_kind::close_paren },
          }));
        }
      }

      SUBCASE("Scientific notation")
      {
        SUBCASE("Valid")
        {
          processor p{ "1e3 -1e2 2.E-3 22.3e-8 -12E+18\\a" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{  0, 1, 1, 3,      token_kind::real,   1000.0 },
                  token{  4, 1, 1, 4,      token_kind::real,   -100.0 },
                  token{  9, 1, 1, 5,      token_kind::real,    0.002 },
                  token{ 15, 1, 1, 7,      token_kind::real, 2.23e-07 },
                  token{ 23, 1, 1, 7,      token_kind::real, -1.2e+19 },
                  token{ 30, 1, 1, 2, token_kind::character,  "\\a"sv },
          }));
        }

        SUBCASE("Missing exponent")
        {
          processor p{ "1e 23E-1 12e- -0.2e" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 2, "unexpected end of real, expecting exponent" },
                  token{ 3, 1, 1, 5, token_kind::real, 2.3 },
                  error{ 9, 13, "unexpected end of real, expecting exponent" },
                  error{ 14, 19, "unexpected end of real, expecting exponent" },
          }));
        }

        SUBCASE("Signs after exponent found")
        {
          processor p{ "12.3 -1e3- 2.3E+" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  token{ 0, 1, 1, 4, token_kind::real, 12.3 },
                  error{ 5, 9, "invalid number" },
                  error{ 9, "expected whitespace before next token" },
                  token{ 9, 1, 1, token_kind::symbol, "-"sv },
                  error{ 11, 16, "unexpected end of real, expecting exponent" },
          }));
        }

        SUBCASE("Extra dots")
        {
          processor p{ "1e3. 12.3 -1e4.3" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 3, "invalid number" },
                  error{ 3, "unexpected character: ." },
                  token{ 5, 1, 1, 4, token_kind::real, 12.3 },
                  error{ 10, 14, "invalid number" },
                  error{ 14, "unexpected character: ." },
                  error{ 15, "expected whitespace before next token" },
                  token{ 15, 1, 1, token_kind::integer, 3ll },
          }));
        }

        SUBCASE("Extra characters in exponent")
        {
          processor p{ "2.ee4 -1e4E3 1.eFoo 3E5fOo" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_results({
                  error{ 0, 3, "invalid number" },
                  token{ 3, 1, 1, 2, token_kind::symbol, "e4"sv },
                  error{ 6, 10, "invalid number" },
                  error{ 10, "expected whitespace before next token" },
                  token{ 10, 1, 1, 2, token_kind::symbol, "E3"sv },
                  error{ 13, 16, "unexpected end of real, expecting exponent" },
                  error{ 16, "expected whitespace before next token" },
                  token{ 16, 1, 1, 3, token_kind::symbol, "Foo"sv },
                  error{ 20, 26, "invalid number: chars 'fOo' are invalid for radix 10" },
          }));
        }
      }
    }

    TEST_CASE("Character")
    {
      SUBCASE("Whitespace after \\")
      {
        processor p{ R"(\ )" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { error(0, "Expecting a valid character literal after \\") } }));
      }

      SUBCASE("Dangling \\")
      {
        processor p{ R"(\)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({ { error(0, "Expecting a valid character literal after \\") } }));
      }

      SUBCASE("Alphabetic")
      {
        processor p{ R"(\a)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::character, "\\a"sv }
        }));
      }

      SUBCASE("Numeric")
      {
        processor p{ R"(\1)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::character, "\\1"sv }
        }));
      }

      SUBCASE("Multiple characters after \\")
      {
        processor p{ R"(\11)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::character, "\\11"sv }
        }));
      }

      SUBCASE("Valid lexed character with symbol")
      {
        processor p{ R"(\1:)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 1, 1, 3, token_kind::character, "\\1:"sv },
        }));
      }

      SUBCASE("Valid consecutive characters")
      {
        processor p{ R"(\1 \newline\' \\)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                {  0, 1, 1, 2, token_kind::character,       "\\1"sv },
                {  3, 1, 1, 8, token_kind::character, "\\newline"sv },
                { 11, 1, 1, 2, token_kind::character,       "\\'"sv },
                { 14, 1, 1, 2, token_kind::character,      "\\\\"sv }
        }));
      }

      SUBCASE("Character followed by a backticked keyword")
      {
        processor p{ R"(\a`:kw)" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                token{ 0, 1, 1, 2, token_kind::character, "\\a"sv },
                token{ 2, 1, 1, token_kind::syntax_quote },
                token{ 3, 1, 1, 3, token_kind::keyword, "kw"sv }
        }));
      }
    }

    TEST_CASE("Symbol")
    {
      SUBCASE("Single-char")
      {
        processor p{ "a" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, token_kind::symbol, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ "abc" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::symbol, "abc"sv }
        }));
      }

      SUBCASE("Single slash")
      {
        processor p{ "/" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, token_kind::symbol, "/"sv }
        }));
      }

      SUBCASE("Multi slash")
      {
        processor p{ "//" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid symbol" },
        }));
      }

      SUBCASE("Starting with a slash")
      {
        processor p{ "/foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid symbol" },
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ "abc123" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 6, token_kind::symbol, "abc123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ "abc_.123/-foo+?=!&<>#%" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 22, token_kind::symbol, "abc_.123/-foo+?=!&<>#%"sv }
        }));
      }

      SUBCASE("Only -")
      {
        processor p{ "-" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, token_kind::symbol, "-"sv }
        }));
      }

      SUBCASE("Starting with -")
      {
        processor p{ "-main" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 5, token_kind::symbol, "-main"sv }
        }));
      }

      SUBCASE("Quoted")
      {
        processor p{ "'foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, token_kind::single_quote },
                { 1, 1, 1, 3, token_kind::symbol, "foo"sv }
        }));
      }
    }

    TEST_CASE("Keyword")
    {
      SUBCASE("Single-char")
      {
        processor p{ ":a" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::keyword, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ ":abc" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 4, token_kind::keyword, "abc"sv }
        }));
      }

      SUBCASE("Whitespace after :")
      {
        processor p{ ": " };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: expected non-whitespace character after :" }
        }));
      }

      SUBCASE("Single slash")
      {
        processor p{ ":/" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::keyword, "/"sv }
        }));
      }

      SUBCASE("Multi slash")
      {
        processor p{ "://" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: starts with /" },
        }));
      }

      SUBCASE("Starting with a slash")
      {
        processor p{ ":/foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: starts with /" },
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ ":abc123" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 7, token_kind::keyword, "abc123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ ":abc_.123/-foo+?=!&<>" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 21, token_kind::keyword, "abc_.123/-foo+?=!&<>"sv }
        }));
      }

      SUBCASE("Auto-resolved unqualified")
      {
        processor p{ "::foo-bar" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 9, token_kind::keyword, ":foo-bar"sv }
        }));
      }

      SUBCASE("Auto-resolved qualified")
      {
        processor p{ "::foo/bar" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 9, token_kind::keyword, ":foo/bar"sv }
        }));
      }

      SUBCASE("Too many starting colons")
      {
        processor p{ ":::foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: incorrect number of :" },
        }));
      }

      SUBCASE("Way too many starting colons")
      {
        processor p{ "::::foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error{ 0, "invalid keyword: incorrect number of :" },
        }));
      }
    }

    TEST_CASE("String")
    {
      SUBCASE("Empty")
      {
        processor p{ "\"\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 2, token_kind::string, ""sv }
        }));
      }
      SUBCASE("Single-char")
      {
        processor p{ "\"a\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 3, token_kind::string, "a"sv }
        }));
      }

      SUBCASE("Multi-char")
      {
        processor p{ "\"abc\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 5, token_kind::string, "abc"sv }
        }));
      }

      SUBCASE("With numbers")
      {
        processor p{ "\"123\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 5, token_kind::string, "123"sv }
        }));
      }

      SUBCASE("With other symbols")
      {
        processor p{ "\"and then() there was abc_123/-foo?!&<>\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 40, token_kind::string, "and then() there was abc_123/-foo?!&<>"sv }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "\"foo\nbar\nspam\t\"" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 15, token_kind::string, "foo\nbar\nspam\t"sv }
        }));
      }

      SUBCASE("With escapes")
      {
        processor p{ R"("foo\"\nbar\nspam\t\r")" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 22, token_kind::escaped_string, "foo\\\"\\nbar\\nspam\\t\\r"sv }
        }));

        processor q{ R"("\??\' \\ a\a b\b f\f v\v")" };
        native_vector<result<token, error>> const tokens2(q.begin(), q.end());
        CHECK(
          tokens2
          == make_tokens({
            { 0, 1, 1, 26, token_kind::escaped_string, "\\\?\?\\' \\\\ a\\a b\\b f\\f v\\v"sv }
        }));
      }

      SUBCASE("Unterminated")
      {
        processor p{ "\"meow" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
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
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::meta_hint }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "^\n:foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::meta_hint },
                { 2, 1, 1, 4, token_kind::keyword, "foo"sv }
        }));
      }
    }

    TEST_CASE("Reader macro")
    {
      SUBCASE("Empty")
      {
        processor p{ "#" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::reader_macro }
        }));
      }

      SUBCASE("Comment")
      {
        SUBCASE("Empty")
        {
          processor p{ "#_" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 2, token_kind::reader_macro_comment }
          }));
        }

        SUBCASE("No whitespace after")
        {
          processor p{ "#_[]" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 2, token_kind::reader_macro_comment },
                  { 2, 1, 1, 1,  token_kind::open_square_bracket },
                  { 3, 1, 1, 1, token_kind::close_square_bracket }
          }));
        }
      }

      SUBCASE("Conditional")
      {
        SUBCASE("Empty")
        {
          processor p{ "#?" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 2, token_kind::reader_macro_conditional }
          }));
        }

        SUBCASE("With following list")
        {
          processor p{ "#?()" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 2, token_kind::reader_macro_conditional },
                  { 2, 1, 1, 1,               token_kind::open_paren },
                  { 3, 1, 1, 1,              token_kind::close_paren }
          }));
        }
      }

      SUBCASE("Set")
      {
        SUBCASE("Empty")
        {
          processor p{ "#{}" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 1,        token_kind::reader_macro },
                  { 1, 1, 1, 1,  token_kind::open_curly_bracket },
                  { 2, 1, 1, 1, token_kind::close_curly_bracket }
          }));
        }

        /* Clojure doesn't actually allow this, but I don't see why not. It does for meta hints, so
         * I figure this is just a lazy inconsistency. */
        SUBCASE("With line breaks")
        {
          processor p{ "#\n{}" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 1,        token_kind::reader_macro },
                  { 2, 1, 1, 1,  token_kind::open_curly_bracket },
                  { 3, 1, 1, 1, token_kind::close_curly_bracket }
          }));
        }
      }
    }

    TEST_CASE("Syntax quoting")
    {
      SUBCASE("Empty")
      {
        processor p{ "`" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::syntax_quote }
        }));
      }

      SUBCASE("With line breaks")
      {
        processor p{ "`\n:foo" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 1, token_kind::syntax_quote },
                { 2, 1, 1, 4, token_kind::keyword, "foo"sv }
        }));
      }

      SUBCASE("Unquote")
      {
        SUBCASE("Empty")
        {
          processor p{ "~" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 1, token_kind::unquote }
          }));
        }

        SUBCASE("With line breaks")
        {
          processor p{ "~\n:foo" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 1, token_kind::unquote },
                  { 2, 1, 1, 4, token_kind::keyword, "foo"sv }
          }));
        }
      }

      SUBCASE("Unquote splicing")
      {
        SUBCASE("Empty")
        {
          processor p{ "~@" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 2, token_kind::unquote_splice }
          }));
        }

        SUBCASE("With line breaks before")
        {
          processor p{ "~\n@:foo" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 1, token_kind::unquote },
                  { 2, 1, 1, 1, token_kind::deref },
                  { 3, 1, 1, 4, token_kind::keyword, "foo"sv }
          }));
        }

        SUBCASE("With line breaks after")
        {
          processor p{ "~@\n:foo" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 2, token_kind::unquote_splice },
                  { 3, 1, 1, 4, token_kind::keyword, "foo"sv }
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
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 4, token_kind::symbol, "👍"sv }
          }));
        }
        SUBCASE("Multiple characters")
        {
          processor p{ "😎👍" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 8, token_kind::symbol, "😎👍"sv }
          }));
        }
        SUBCASE("Symbol with mixed characters inside")
        {
          processor p{ "one-🍺-please!" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 16, token_kind::symbol, "one-🍺-please!"sv }
          }));
        }
        SUBCASE("Qualified Symbol")
        {
          processor p{ "🐝/🥀" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 9, token_kind::symbol, "🐝/🥀"sv }
          }));
        }
      }
      SUBCASE("Keywords")
      {
        SUBCASE("Single character")
        {
          processor p{ ":🍙" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 5, token_kind::keyword, "🍙"sv },
          }));
        }
        SUBCASE("Multiple characters keyword")
        {
          processor p{ ":🥩🍗" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 9, token_kind::keyword, "🥩🍗"sv }
          }));
        }
        SUBCASE("Keyword with mixed characters inside")
        {
          processor p{ ":w🍪w" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 7, token_kind::keyword, "w🍪w"sv }
          }));
        }
        SUBCASE("Qualified Keyword")
        {
          processor p{ ":🐝/🥀" };
          native_vector<result<token, error>> const tokens(p.begin(), p.end());
          CHECK(tokens
                == make_tokens({
                  { 0, 1, 1, 10, token_kind::keyword, "🐝/🥀"sv }
          }));
        }
      }
      SUBCASE("8-wide character")
      {
        processor p{ "🇪🇺" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 8, token_kind::symbol, "🇪🇺"sv }
        }));
      }
      SUBCASE("Non-Latin Characters")
      {
        processor p{ "ありがとう" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 15, token_kind::symbol, "ありがとう"sv }
        }));
      }
      SUBCASE("Non-Latin Characters")
      {
        processor p{ ":ありがとう" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 16, token_kind::keyword, "ありがとう"sv }
        }));
      }
      SUBCASE("Whitespace Characters")
      {
        processor p{ ":  " };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_tokens({
                { 0, 1, 1, 7, token_kind::keyword, "  "sv }
        }));
      }


      SUBCASE("Malformed Text")
      {
        processor p{ "\xC0\x80" };
        native_vector<result<token, error>> const tokens(p.begin(), p.end());
        CHECK(tokens
              == make_results({
                error({ 0, "Unfinished Character" }),
                error({ 1, "Unfinished Character" }),
              }));
      }
    }
  }
}
