#include <unistd.h>

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
#include <doctest/doctest.h>

namespace jank::read::parse
{
  TEST_CASE("Empty")
  {
    runtime::context rt_ctx;
    lex::processor lp{ "" };
    processor p{ rt_ctx, lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok() == nullptr);
  }

  TEST_CASE("Nil")
  {
    lex::processor lp{ "nil" };
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::nil{ }));
  }

  TEST_CASE("Boolean")
  {
    lex::processor lp{ "true false" };
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
    auto const t(p.next());
    CHECK(t.is_ok());
    CHECK(t.expect_ok()->equal(runtime::obj::boolean{ true }));
    auto const f(p.next());
    CHECK(f.expect_ok()->equal(runtime::obj::boolean{ false }));
  }

  TEST_CASE("Integer")
  {
    lex::processor lp{ "1234" };
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::integer{ 1234 }));
  }

  TEST_CASE("Comments")
  {
    lex::processor lp{ ";meow \n1234 ; bar\n;\n\n" };
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
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
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
    auto const r(p.next());
    CHECK(r.is_ok());
    CHECK(r.expect_ok()->equal(runtime::obj::real{ 12.34l }));
  }

  TEST_CASE("String")
  {
    lex::processor lp{R"("foo" "bar")"};
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
    for(auto const &s : { "foo", "bar" })
    {
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok()->equal(runtime::obj::string{ native_string{ s } }));
    }
  }

  TEST_CASE("Symbol")
  {
    SUBCASE("Unqualified")
    {
      lex::processor lp{ "foo bar spam" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(auto const &s : { "foo", "bar", "spam" })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(runtime::obj::symbol{ "", native_string{ s } }));
      }
    }

    SUBCASE("Slash")
    {
      lex::processor lp{ "/" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok()->equal(runtime::obj::symbol{ "", native_string{ "/" } }));
    }

    SUBCASE("Qualified")
    {
      lex::processor lp{ "foo/foo foo.bar/bar spam.bar/spam" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
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
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("", "foo"), std::make_pair("bar", "spam") })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            jank::make_box<runtime::obj::list>
            (
              make_box<runtime::obj::symbol>("quote"),
              make_box<runtime::obj::symbol>(s.first, s.second)
            )
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
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(auto const &s : { "foo", "bar", "spam" })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(rt_ctx.intern_keyword(runtime::obj::symbol{ "", native_string{ s } }, true)));
      }
    }

    SUBCASE("Qualified")
    {
      lex::processor lp{ ":foo/foo :foo.bar/bar :spam.bar/spam" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(auto const &s : { std::make_pair("foo", "foo"), std::make_pair("foo.bar", "bar"), std::make_pair("spam.bar", "spam") })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(rt_ctx.intern_keyword(runtime::obj::symbol{ s.first, s.second }, true)));
      }
    }

    SUBCASE("Auto-resolved unqualified")
    {
      lex::processor lp{ "::foo ::spam" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(auto const &s : { "foo", "spam" })
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok()->equal(rt_ctx.intern_keyword(runtime::obj::symbol{ "", native_string{ s } }, false)));
      }
    }

  }

  /* TODO: Move this into a subcase once it's passing. */
  TEST_CASE("Auto-resolved qualified" * doctest::skip())
  {
    lex::processor lp{ "::foo/foo ::foo.bar/bar ::spam.bar/spam" };
    runtime::context rt_ctx;
    processor p{ rt_ctx, lp.begin(), lp.end() };
    for(auto const &s : { std::make_pair("foo", "foo"), std::make_pair("foo.bar", "bar"), std::make_pair("spam.bar", "spam") })
    {
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok()->equal(rt_ctx.intern_keyword(runtime::obj::symbol{ s.first, s.second }, false)));
    }
  }

  TEST_CASE("List")
  {
    SUBCASE("Empty")
    {
      lex::processor lp{ "() ( ) (   )" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(size_t i{}; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(jank::make_box<runtime::obj::list>()));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "(1 2 3 4) ( 2, 4 6, 8 )" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            jank::make_box<runtime::obj::list>
            (
              make_box<runtime::obj::integer>(1 * i),
              make_box<runtime::obj::integer>(2 * i),
              make_box<runtime::obj::integer>(3 * i),
              make_box<runtime::obj::integer>(4 * i)
            )
          )
        );
      }
    }

    SUBCASE("Mixed")
    {
      lex::processor lp{ "(def foo-bar 1) foo-bar" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK
      (
        r1.expect_ok()->equal
        (
          jank::make_box<runtime::obj::list>
          (
            make_box<runtime::obj::symbol>("def"),
            make_box<runtime::obj::symbol>("foo-bar"),
            make_box<runtime::obj::integer>(1)
          )
        )
      );
      auto const r2(p.next());
      CHECK(r2.is_ok());
      CHECK(r2.expect_ok()->equal(make_box<runtime::obj::symbol>("foo-bar")));
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ "1)" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::obj::integer{ 1 }));
      auto const r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "(1" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }
  }

  TEST_CASE("Vector")
  {
    SUBCASE("Empty")
    {
      lex::processor lp{ "[] [ ] [   ]" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(size_t i{}; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(r.expect_ok() != nullptr);
        CHECK(r.expect_ok()->equal(make_box<runtime::obj::vector>()));
      }
    }

    SUBCASE("Non-empty")
    {
      lex::processor lp{ "[1 2 3 4] [ 2, 4 6, 8 ]" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      for(size_t i{ 1 }; i < 3; ++i)
      {
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK
        (
          r.expect_ok()->equal
          (
            runtime::obj::vector::create
            (
              runtime::detail::peristent_vector
              {
                make_box<runtime::obj::integer>(1 * i),
                make_box<runtime::obj::integer>(2 * i),
                make_box<runtime::obj::integer>(3 * i),
                make_box<runtime::obj::integer>(4 * i),
              }
            )
          )
        );
      }
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ "1]" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(runtime::obj::integer{ 1 }));
      auto const r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "[1" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }
  }

  TEST_CASE("Map")
  {
    SUBCASE("Empty")
    {
      lex::processor lp{ "{} { } {,,,}" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
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
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
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
              runtime::detail::in_place_unique{},
              runtime::obj::map::value_type::value_type{
                {
                  make_box<runtime::obj::integer>(1 * i),
                  make_box<runtime::obj::integer>(2 * i),
                },
                {
                  make_box<runtime::obj::integer>(3 * i),
                  make_box<runtime::obj::integer>(4 * i),
                }
              }
            }
          )
        );
      }
    }

    SUBCASE("Heterogeneous")
    {
      lex::processor lp{R"({:foo true 1 :one "meow" "meow"})"};
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok() != nullptr);
      CHECK
      (
        r.expect_ok()->equal
        (
          runtime::obj::map
          {
            runtime::detail::in_place_unique{},
            runtime::obj::map::value_type::value_type{
              {
                rt_ctx.intern_keyword(runtime::obj::symbol{ "foo" }, true),
                make_box<runtime::obj::boolean>(true),
              },
              {
                make_box<runtime::obj::integer>(1),
                rt_ctx.intern_keyword(runtime::obj::symbol{ "one" }, true),
              },
              {
                make_box<runtime::obj::string>("meow"),
                make_box<runtime::obj::string>("meow"),
              }
            }
          }
        )
      );
    }

    SUBCASE("Odd elements")
    {
      lex::processor lp{ "{1 2 3}" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }

    SUBCASE("Extra close")
    {
      lex::processor lp{ ":foo}" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_ok());
      CHECK(r1.expect_ok()->equal(rt_ctx.intern_keyword(runtime::obj::symbol{ "foo" }, true)));
      auto const r2(p.next());
      CHECK(r2.is_err());
    }

    SUBCASE("Unterminated")
    {
      lex::processor lp{ "{1" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r1(p.next());
      CHECK(r1.is_err());
    }
  }
}
