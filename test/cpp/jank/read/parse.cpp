#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <unistd.h>
#pragma clang diagnostic pop

#include <vector>
#include <array>
#include <iostream>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/list.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
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

  TEST_CASE("Nil")
  {
    lex::processor lp{ "nil" };
    processor p{ lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::nil{ }));
  }

  TEST_CASE("Boolean")
  {
    lex::processor lp{ "true false" };
    processor p{ lp.begin(), lp.end() };
    auto const t(p.next());
    CHECK(t.is_ok());
    CHECK(t.expect_ok()->equal(runtime::obj::boolean{ true }));
    auto const f(p.next());
    CHECK(f.expect_ok()->equal(runtime::obj::boolean{ false }));
  }

  TEST_CASE("Integer")
  {
    lex::processor lp{ "1234" };
    processor p{ lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::integer{ 1234 }));
  }

  TEST_CASE("Comments")
  {
    lex::processor lp{ ";meow \n1234 ; bar\n;\n\n" };
    processor p{ lp.begin(), lp.end() };
    auto const i(p.next());
    CHECK(i.is_ok());
    CHECK(i.expect_ok()->equal(runtime::obj::integer{ 1234 }));

    auto const eof(p.next());
    CHECK(eof.is_ok());
    CHECK(eof.expect_ok() == nullptr);
  }

  TEST_CASE("Real")
  {
    lex::processor lp{ "12.34" };
    processor p{ lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::real{ 12.34l }));
  }

  TEST_CASE("String")
  {
    lex::processor lp{R"("foo" "bar")"};
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
        CHECK(r.expect_ok()->equal(runtime::obj::symbol{ "", runtime::detail::string_type{ s } }));
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

  TEST_CASE("Keyword")
  {
    SUBCASE("Unqualified")
    {
      lex::processor lp{ ":foo :bar :spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { "foo", "bar", "spam" })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::keyword{ runtime::obj::symbol{ "", runtime::detail::string_type{ s } }, true }));
      }
    }

    SUBCASE("Qualified")
    {
      lex::processor lp{ ":foo/foo :foo.bar/bar :spam.bar/spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("foo", "foo"), std::make_pair("foo.bar", "bar"), std::make_pair("spam.bar", "spam") })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::keyword{ runtime::obj::symbol{ s.first, s.second }, true }));
      }
    }

    SUBCASE("Auto-resolved unqualified")
    {
      lex::processor lp{ "::foo ::spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { "foo", "spam" })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::keyword{ runtime::obj::symbol{ "", runtime::detail::string_type{ s } }, false }));
      }
    }

    SUBCASE("Auto-resolved qualified")
    {
      lex::processor lp{ "::foo/foo ::foo.bar/bar ::spam.bar/spam" };
      processor p{ lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("foo", "foo"), std::make_pair("foo.bar", "bar"), std::make_pair("spam.bar", "spam") })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::keyword{ runtime::obj::symbol{ s.first, s.second }, false }));
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

  TEST_CASE("Map")
  {
    SUBCASE("Empty")
    {
      lex::processor lp{ "{} { } {,,,}" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{}; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(runtime::obj::map{}));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "{1 2 3 4} { 2, 4 6, 8 }" };
      processor p{ lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::obj::map
            {
              std::in_place,
              runtime::make_box<runtime::obj::integer>(1 * i),
              runtime::make_box<runtime::obj::integer>(2 * i),
              runtime::make_box<runtime::obj::integer>(3 * i),
              runtime::make_box<runtime::obj::integer>(4 * i),
            }
          )
        );
      }
    }

    SUBCASE("Heterogeneous")
    {
      lex::processor lp{R"({:foo true 1 :one "meow" "meow"})"};
      processor p{ lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok() != nullptr);
      CHECK
      (
        r.expect_ok()->equal
        (
          runtime::obj::map
          {
            std::in_place,
            runtime::make_box<runtime::obj::keyword>(runtime::obj::symbol{ "foo" }, true),
            runtime::make_box<runtime::obj::boolean>(true),
            runtime::make_box<runtime::obj::integer>(1),
            runtime::make_box<runtime::obj::keyword>(runtime::obj::symbol{ "one" }, true),
            runtime::make_box<runtime::obj::string>("meow"),
            runtime::make_box<runtime::obj::string>("meow"),
          }
        )
      );
    }

    SUBCASE("Odd elements")
    {
      lex::processor lp{ "{1 2 3}" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ ":foo}" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::obj::keyword{ runtime::obj::symbol{ "foo" }, true }));
      auto const r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "{1" };
      processor p{ lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }
  }
}
