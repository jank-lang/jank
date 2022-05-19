#include <doctest/doctest.h>
#include <unistd.h>

#include <vector>
#include <array>
#include <iostream>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/runtime/type/number.hpp>
#include <jank/runtime/type/symbol.hpp>
#include <jank/runtime/type/vector.hpp>
#include <jank/runtime/type/string.hpp>
#include <jank/runtime/type/list.hpp>

namespace jank::read::parse
{
  TEST_CASE("Empty")
  {
    lex::processor lp{ "" };
    processor p{ lp.begin(), lp.end() };
    auto r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok() == nullptr);
  }

  TEST_CASE("Integer")
  {
    lex::processor lp{ "1234" };
    processor p{ lp.begin(), lp.end() };
    auto r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::type::integer{ 1234 }));
  }

  TEST_CASE("String")
  {
    lex::processor lp{ "\"foo\" \"bar\"" };
    processor p{ lp.begin(), lp.end() };
    for(auto const &s : { "foo", "bar" })
    {
      auto r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok()->equal(runtime::type::string{ runtime::detail::string_type{ s } }));
    }
  }

  TEST_CASE("Symbol")
  {
    SUBCASE("Unqualified")
    {
      lex::processor lp{ "foo bar spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { "foo", "bar", "spam" })
      {
        auto r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::type::symbol{ runtime::detail::string_type{ s } }));
      }
    }

    SUBCASE("Qualified")
    {
      lex::processor lp{ "foo/foo foo.bar/bar spam.bar/spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("foo", "foo"), std::make_pair("foo.bar", "bar"), std::make_pair("spam.bar", "spam") })
      {
        auto r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::type::symbol{ s.first, s.second }));
      }
    }
  }

  TEST_CASE("List")
  {
    SUBCASE("Empty")
    {
      lex::processor lp{ "() ( ) (   )" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{}; i < 3; ++i)
      {
        auto r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(runtime::type::list{}));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "(1 2 3 4) ( 2, 4 6, 8 )" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::type::list
            {
              runtime::detail::list_type
              {
                runtime::make_box<runtime::type::integer>(1 * i),
                runtime::make_box<runtime::type::integer>(2 * i),
                runtime::make_box<runtime::type::integer>(3 * i),
                runtime::make_box<runtime::type::integer>(4 * i),
              }
            }
          )
        );
      }
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ "1)" };
      processor p{ lp.begin(), lp.end() };
      auto r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::type::integer{ 1 }));
      auto r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "(1" };
      processor p{ lp.begin(), lp.end() };
      auto r1(p.next());
      CHECK(r1.is_err());
    }
  }

  TEST_CASE("Vector")
  {
    SUBCASE("Empty")
    {
      lex::processor lp{ "[] [ ] [   ]" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{}; i < 3; ++i)
      {
        auto r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(runtime::type::vector{}));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "[1 2 3 4] [ 2, 4 6, 8 ]" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::type::vector
            {
              runtime::detail::vector_type
              {
                runtime::make_box<runtime::type::integer>(1 * i),
                runtime::make_box<runtime::type::integer>(2 * i),
                runtime::make_box<runtime::type::integer>(3 * i),
                runtime::make_box<runtime::type::integer>(4 * i),
              }
            }
          )
        );
      }
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ "1]" };
      processor p{ lp.begin(), lp.end() };
      auto r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::type::integer{ 1 }));
      auto r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "[1" };
      processor p{ lp.begin(), lp.end() };
      auto r1(p.next());
      CHECK(r1.is_err());
    }
  }
}
