#include <unistd.h>

#include <vector>
#include <array>
#include <iostream>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/list.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

namespace jank::read::parse
{
  TEST_CASE("Empty")
  {
    lex::processor lp{ "" };
    processor p{ lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok() == nullptr);
  }

  TEST_CASE("Integer")
  {
    lex::processor lp{ "1234" };
    processor p{ lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::integer{ 1234 }));
  }

  TEST_CASE("String")
  {
    lex::processor lp{ "\"foo\" \"bar\"" };
    processor p{ lp.begin(), lp.end() };
    for(auto const &s : { "foo", "bar" })
    {
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok()->equal(runtime::obj::string{ runtime::detail::string_type{ s } }));
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
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::symbol{ runtime::detail::string_type{ s } }));
      }
    }

    SUBCASE("Qualified")
    {
      lex::processor lp{ "foo/foo foo.bar/bar spam.bar/spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("foo", "foo"), std::make_pair("foo.bar", "bar"), std::make_pair("spam.bar", "spam") })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::symbol{ s.first, s.second }));
      }
    }

    SUBCASE("Quoted")
    {
      lex::processor lp{ "'foo 'bar/spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("", "foo"), std::make_pair("bar", "spam") })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::obj::list
            {
              runtime::obj::symbol::create("quote"),
              runtime::obj::symbol::create(s.first, s.second)
            }
          )
        );
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
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(runtime::obj::list{}));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "(1 2 3 4) ( 2, 4 6, 8 )" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::obj::list
            {
              runtime::make_box<runtime::obj::integer>(1 * i),
              runtime::make_box<runtime::obj::integer>(2 * i),
              runtime::make_box<runtime::obj::integer>(3 * i),
              runtime::make_box<runtime::obj::integer>(4 * i),
            }
          )
        );
      }
    }

    SUBCASE("Mixed")
    {
      lex::processor lp{ "(def foo-bar 1) foo-bar" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK
      (
        r1.expect_ok()->equal
        (
          runtime::obj::list
          {
            runtime::make_box<runtime::obj::symbol>("def"),
            runtime::make_box<runtime::obj::symbol>("foo-bar"),
            runtime::make_box<runtime::obj::integer>(1),
          }
        )
      );
      auto const r2(p.next());
      CHECK(r2.is_ok());
      CHECK(r2.expect_ok()->equal(runtime::make_box<runtime::obj::symbol>("foo-bar")));
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ "1)" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::obj::integer{ 1 }));
      auto const r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "(1" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
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
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(runtime::obj::vector{}));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "[1 2 3 4] [ 2, 4 6, 8 ]" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::obj::vector
            {
              runtime::detail::vector_type
              {
                runtime::make_box<runtime::obj::integer>(1 * i),
                runtime::make_box<runtime::obj::integer>(2 * i),
                runtime::make_box<runtime::obj::integer>(3 * i),
                runtime::make_box<runtime::obj::integer>(4 * i),
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
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::obj::integer{ 1 }));
      auto const r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "[1" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }
  }
}
