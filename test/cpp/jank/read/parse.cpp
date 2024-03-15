#include <unistd.h>

#include <array>
#include <iostream>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/detail/object_util.hpp>
#include <jank/util/escape.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::read::parse
{
  TEST_SUITE("parse")
  {
    TEST_CASE("Empty")
    {
      runtime::context rt_ctx;
      lex::processor lp{ "" };
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(r.expect_ok().is_none());
    }

    TEST_CASE("Nil")
    {
      lex::processor lp{ "nil" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, runtime::obj::nil::nil_const()));
      CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 3, lex::token_kind::nil });
      CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
    }

    TEST_CASE("Boolean")
    {
      lex::processor lp{ "true false" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const t(p.next());
      CHECK(t.is_ok());
      CHECK(runtime::detail::equal(t.expect_ok().unwrap().ptr, make_box(true)));
      CHECK(t.expect_ok().unwrap().start == lex::token{ 0, 4, lex::token_kind::boolean, true });
      CHECK(t.expect_ok().unwrap().end == t.expect_ok().unwrap().start);

      auto const f(p.next());
      CHECK(runtime::detail::equal(f.expect_ok().unwrap().ptr, make_box(false)));
      CHECK(f.expect_ok().unwrap().start == lex::token{ 5, 5, lex::token_kind::boolean, false });
      CHECK(f.expect_ok().unwrap().end == f.expect_ok().unwrap().start);
    }

    TEST_CASE("Integer")
    {
      lex::processor lp{ "1234" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(1234)));
      CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 4, lex::token_kind::integer, 1234ll });
      CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
    }

    TEST_CASE("Comments")
    {
      lex::processor lp{ ";meow \n1234 ; bar\n;\n\n" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const i(p.next());
      CHECK(i.is_ok());
      CHECK(runtime::detail::equal(i.expect_ok().unwrap().ptr, make_box(1234)));

      auto const eof(p.next());
      CHECK(eof.is_ok());
      CHECK(eof.expect_ok().is_none());
    }

    TEST_CASE("Real")
    {
      lex::processor lp{ "12.34" };
      runtime::context rt_ctx;
      processor p{ rt_ctx, lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.is_ok());
      CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(12.34l)));
      CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 5, lex::token_kind::real, 12.34l });
      CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
    }

    TEST_CASE("String")
    {
      SUBCASE("Unescaped")
      {
        lex::processor lp{ R"("foo" "bar")" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "bar" })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(s)));

          /* We add 2 for the surrounding quotes. */
          auto const len(strlen(s) + 2);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::string, s });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each string is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Escaped")
      {
        lex::processor lp{ R"("foo\n" "\t\"bar\"")" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo\n", "\t\"bar\"" })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(s)));

          /* We add 2 for the surrounding quotes. */
          auto const escaped(util::escape(s));
          auto const len(escaped.size() + 2);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::escaped_string, escaped });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each string is 1 space apart. */
          offset += len + 1;
        }
      }
    }

    TEST_CASE("Symbol")
    {
      SUBCASE("Unqualified")
      {
        lex::processor lp{ "foo bar spam" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "bar", "spam" })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::symbol>("", s)));

          auto const len(strlen(s));
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::symbol, s });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each symbol is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Slash")
      {
        lex::processor lp{ "/" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     make_box<runtime::obj::symbol>("", "/")));
        CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 1, lex::token_kind::symbol, "/" });
        CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
      }

      SUBCASE("Qualified")
      {
        lex::processor lp{ "foo/foo foo.bar/bar spam.bar/spam" };
        runtime::context rt_ctx;
        rt_ctx.intern_ns(make_box<runtime::obj::symbol>("foo"));
        rt_ctx.intern_ns(make_box<runtime::obj::symbol>("foo.bar"));
        rt_ctx.intern_ns(make_box<runtime::obj::symbol>("spam.bar"));
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { std::make_pair("foo", "foo"),
                              std::make_pair("foo.bar", "bar"),
                              std::make_pair("spam.bar", "spam") })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::symbol>(s.first, s.second)));

          /* We add one for the slash. */
          auto const len(strlen(s.first) + strlen(s.second) + 1);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset,
                               len,
                               lex::token_kind::symbol,
                               fmt::format("{}/{}", s.first, s.second) });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each symbol is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Qualified, aliased")
      {
        lex::processor lp{ "foo.bar/bar" };
        runtime::context rt_ctx;
        auto const meow(rt_ctx.intern_ns(make_box<runtime::obj::symbol>("meow")));
        rt_ctx.current_ns()->add_alias(make_box<runtime::obj::symbol>("foo.bar"), meow).expect_ok();
        processor p{ rt_ctx, lp.begin(), lp.end() };
        for(auto const &s : { std::make_pair("meow", "bar") })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::symbol>(s.first, s.second)));
        }
      }

      SUBCASE("Qualified, non-existent ns")
      {
        lex::processor lp{ "foo.bar/bar" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_err());
      }

      SUBCASE("Quoted")
      {
        lex::processor lp{ "'foo 'bar/spam 'foo.bar/bar" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { std::make_pair("", "foo"),
                              std::make_pair("bar", "spam"),
                              std::make_pair("foo.bar", "bar") })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::persistent_list>(
                                         make_box<runtime::obj::symbol>("quote"),
                                         make_box<runtime::obj::symbol>(s.first, s.second))));

          auto const ns_len(strlen(s.first));
          /* We add one for the slash. */
          auto const len(strlen(s.first) + strlen(s.second) + (ns_len == 0 ? 0 : 1));
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::single_quote });
          offset += 1;
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset,
                               len,
                               lex::token_kind::symbol,
                               (ns_len == 0 ? fmt::format("{}", s.second)
                                            : fmt::format("{}/{}", s.first, s.second)) });
          /* Each symbol is 1 space apart. */
          offset += len + 1;
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

        size_t offset{};
        for(auto const &s : { "foo", "bar", "spam" })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            rt_ctx.intern_keyword(runtime::obj::symbol{ "", s }, true).expect_ok()));

          /* We add one for the colon. */
          auto const len(strlen(s) + 1);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::keyword, s });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each symbol is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Qualified")
      {
        lex::processor lp{ ":foo/foo :foo.bar/bar :spam.bar/spam" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { std::make_pair("foo", "foo"),
                              std::make_pair("foo.bar", "bar"),
                              std::make_pair("spam.bar", "spam") })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            rt_ctx.intern_keyword(runtime::obj::symbol{ s.first, s.second }, true).expect_ok()));

          /* We add one for the colon and one for the slash. */
          auto const len(strlen(s.first) + strlen(s.second) + 2);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset,
                               len,
                               lex::token_kind::keyword,
                               fmt::format("{}/{}", s.first, s.second) });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each symbol is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Auto-resolved unqualified")
      {
        lex::processor lp{ "::foo ::spam" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "spam" })
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            rt_ctx.intern_keyword(runtime::obj::symbol{ "", native_persistent_string{ s } }, false)
              .expect_ok()));

          /* We add one for each colon. */
          auto const len(strlen(s) + 2);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::keyword, fmt::format(":{}", s) });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
          /* Each keyword is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Auto-resolved qualified, missing alias")
      {
        lex::processor lp{ "::foo/foo" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_err());
      }

      SUBCASE("Auto-resolved qualified, with alias")
      {
        lex::processor lp{ "::foo/foo" };
        runtime::context rt_ctx;
        auto const foo_ns(rt_ctx.intern_ns(make_box<runtime::obj::symbol>("foo.bar.spam")));
        auto const clojure_ns(rt_ctx.find_ns(make_box<runtime::obj::symbol>("clojure.core")));
        clojure_ns.unwrap()->add_alias(make_box<runtime::obj::symbol>("foo"), foo_ns).expect_ok();
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(runtime::detail::equal(
          r.expect_ok().unwrap().ptr,
          rt_ctx.intern_keyword(runtime::obj::symbol{ "foo.bar.spam", "foo" }, true).expect_ok()));
        CHECK(r.expect_ok().unwrap().start
              == lex::token{ 0, 9, lex::token_kind::keyword, ":foo/foo" });
        CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
      }
    }

    TEST_CASE("List")
    {
      SUBCASE("Empty")
      {
        lex::processor lp{ "() ( ) (  )" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{}; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       runtime::obj::persistent_list::empty()));

          /* We add one for each paren. */
          auto const len(2 + i);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::open_paren });
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset + i + 1, 1, lex::token_kind::close_paren });
          /* Each list is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Non-empty")
      {
        lex::processor lp{ "(1, 2, 3, 4) ( 2, 4, 6, 8 )" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{ 1 }; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_list>(make_box<runtime::obj::integer>(1 * i),
                                                    make_box<runtime::obj::integer>(2 * i),
                                                    make_box<runtime::obj::integer>(3 * i),
                                                    make_box<runtime::obj::integer>(4 * i))));

          /* Parens, nums, spaces, commas, and optionally more spaces. */
          auto const len(2 + 4 + 3 + 3 + (i == 2 ? 2 : 0));
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::open_paren });
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset + len - 1, 1, lex::token_kind::close_paren });
          /* Each list is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Mixed")
      {
        lex::processor lp{ "(def foo-bar 1) foo-bar" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_ok());
        CHECK(runtime::detail::equal(
          r1.expect_ok().unwrap().ptr,
          make_box<runtime::obj::persistent_list>(make_box<runtime::obj::symbol>("def"),
                                                  make_box<runtime::obj::symbol>("foo-bar"),
                                                  make_box<runtime::obj::integer>(1))));
        auto const r2(p.next());
        CHECK(r2.is_ok());
        CHECK(runtime::detail::equal(r2.expect_ok().unwrap().ptr,
                                     make_box<runtime::obj::symbol>("foo-bar")));
      }

      SUBCASE("Extra close")
      {
        lex::processor lp{ "1)" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_ok());
        CHECK(runtime::detail::equal(r1.expect_ok().unwrap().ptr, make_box(1)));
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
        lex::processor lp{ "[] [ ] [  ]" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{}; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::persistent_vector>()));

          /* We add one for each bracket. */
          auto const len(2 + i);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::open_square_bracket });
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset + i + 1, 1, lex::token_kind::close_square_bracket });
          /* Each vector is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Non-empty")
      {
        lex::processor lp{ "[1, 2, 3, 4] [ 2, 4, 6, 8 ]" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{ 1 }; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_vector>(runtime::detail::native_persistent_vector{
              make_box<runtime::obj::integer>(1 * i),
              make_box<runtime::obj::integer>(2 * i),
              make_box<runtime::obj::integer>(3 * i),
              make_box<runtime::obj::integer>(4 * i),
            })));

          /* Brackets, nums, spaces, commas, and optionally more spaces. */
          auto const len(2 + 4 + 3 + 3 + (i == 2 ? 2 : 0));
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::open_square_bracket });
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset + len - 1, 1, lex::token_kind::close_square_bracket });
          /* Each vector is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Extra close")
      {
        lex::processor lp{ "1]" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_ok());
        CHECK(runtime::detail::equal(r1.expect_ok().unwrap().ptr, make_box(1)));
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
        lex::processor lp{ "{} { } {,,}" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{}; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::persistent_array_map>()));

          /* We add one for each bracket. */
          auto const len(2 + i);
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::open_curly_bracket });
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset + i + 1, 1, lex::token_kind::close_curly_bracket });
          /* Each map is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Non-empty")
      {
        lex::processor lp{ "{1 2, 3 4} { 2 4, 6 8 }" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{ 1 }; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(r.is_ok());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_array_map>(
              runtime::detail::in_place_unique{},
              make_array_box<runtime::object_ptr>(make_box<runtime::obj::integer>(1 * i),
                                                  make_box<runtime::obj::integer>(2 * i),
                                                  make_box<runtime::obj::integer>(3 * i),
                                                  make_box<runtime::obj::integer>(4 * i)),
              4)));

          /* Brackets, nums, spaces, commas, and optionally more spaces. */
          auto const len(2 + 4 + 3 + 1 + (i == 2 ? 2 : 0));
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 1, lex::token_kind::open_curly_bracket });
          CHECK(r.expect_ok().unwrap().end
                == lex::token{ offset + len - 1, 1, lex::token_kind::close_curly_bracket });
          /* Each map is 1 space apart. */
          offset += len + 1;
        }
      }

      SUBCASE("Heterogeneous")
      {
        lex::processor lp{ R"({:foo true 1 :one "meow" "meow"})" };
        runtime::context rt_ctx;
        processor p{ rt_ctx, lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_ok());
        CHECK(runtime::detail::equal(
          r.expect_ok().unwrap().ptr,
          make_box<runtime::obj::persistent_array_map>(
            runtime::detail::in_place_unique{},
            make_array_box<runtime::object_ptr>(
              rt_ctx.intern_keyword(runtime::obj::symbol{ "foo" }, true).expect_ok(),
              make_box<runtime::obj::boolean>(true),
              make_box<runtime::obj::integer>(1),
              rt_ctx.intern_keyword(runtime::obj::symbol{ "one" }, true).expect_ok(),
              make_box<runtime::obj::persistent_string>("meow"),
              make_box<runtime::obj::persistent_string>("meow")),
            6)));
        CHECK(r.expect_ok().unwrap().start
              == lex::token{ 0, 1, lex::token_kind::open_curly_bracket });
        CHECK(r.expect_ok().unwrap().end
              == lex::token{ 31, 1, lex::token_kind::close_curly_bracket });
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
        CHECK(runtime::detail::equal(
          r1.expect_ok().unwrap().ptr,
          rt_ctx.intern_keyword(runtime::obj::symbol{ "foo" }, true).expect_ok()));
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
}
