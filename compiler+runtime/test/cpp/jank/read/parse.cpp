#include <unistd.h>

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
  using runtime::__rt_ctx;

  TEST_SUITE("parse")
  {
    TEST_CASE("Empty")
    {
      lex::processor lp{ "" };
      processor p{ lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(r.expect_ok().is_none());
    }

    TEST_CASE("Nil")
    {
      lex::processor lp{ "nil" };
      processor p{ lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, runtime::obj::nil::nil_const()));
      CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 3, lex::token_kind::nil });
      CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
    }

    TEST_CASE("Boolean")
    {
      lex::processor lp{ "true false" };
      processor p{ lp.begin(), lp.end() };
      auto const t(p.next());
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
      processor p{ lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(1234)));
      CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 4, lex::token_kind::integer, 1234ll });
      CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
    }

    TEST_CASE("Comments")
    {
      lex::processor lp{ ";meow \n1234 ; bar\n;\n\n" };
      processor p{ lp.begin(), lp.end() };
      auto const i(p.next());
      CHECK(runtime::detail::equal(i.expect_ok().unwrap().ptr, make_box(1234)));

      auto const eof(p.next());
      CHECK(eof.expect_ok().is_none());
    }

    TEST_CASE("Real")
    {
      lex::processor lp{ "12.34" };
      processor p{ lp.begin(), lp.end() };
      auto const r(p.next());
      CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(12.34l)));
      CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 5, lex::token_kind::real, 12.34l });
      CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
    }

    TEST_CASE("Character")
    {
      SUBCASE("Single")
      {
        lex::processor lp{ R"(\a\1\`\:\#)" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(native_persistent_string const &ch : { "\\a", "\\1", "\\`", "\\:", "\\#" })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::character>(ch)));

          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, 2, lex::token_kind::character, ch });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);

          /* Current character and then a backslash */
          offset += 2;
        }
      }

      SUBCASE("Special")
      {
        lex::processor lp{ R"(\newline \backspace \return \formfeed \tab \space)" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(native_persistent_string const &ch :
            { "\\newline", "\\backspace", "\\return", "\\formfeed", "\\tab", "\\space" })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::character>(ch)));

          auto const len(ch.size());
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::character, ch });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);

          /* +1 for space */
          offset += len + 1;
        }
      }

      SUBCASE("Special and single")
      {
        lex::processor lp{ R"(\newline\a\tab\`\space)" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(native_persistent_string const &ch : { "\\newline", "\\a", "\\tab", "\\`", "\\space" })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::character>(ch)));

          auto const len(ch.size());
          CHECK(r.expect_ok().unwrap().start
                == lex::token{ offset, len, lex::token_kind::character, ch });
          CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);

          offset += len;
        }
      }
    }

    TEST_CASE("String")
    {
      SUBCASE("Unescaped")
      {
        lex::processor lp{ R"("foo" "bar")" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "bar" })
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo\n", "\t\"bar\"" })
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "bar", "spam" })
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     make_box<runtime::obj::symbol>("", "/")));
        CHECK(r.expect_ok().unwrap().start == lex::token{ 0, 1, lex::token_kind::symbol, "/" });
        CHECK(r.expect_ok().unwrap().end == r.expect_ok().unwrap().start);
      }

      SUBCASE("Qualified")
      {
        lex::processor lp{ "foo/foo foo.bar/bar spam.bar/spam" };
        __rt_ctx->intern_ns(make_box<runtime::obj::symbol>("foo"));
        __rt_ctx->intern_ns(make_box<runtime::obj::symbol>("foo.bar"));
        __rt_ctx->intern_ns(make_box<runtime::obj::symbol>("spam.bar"));
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { std::make_pair("foo", "foo"),
                              std::make_pair("foo.bar", "bar"),
                              std::make_pair("spam.bar", "spam") })
        {
          auto const r(p.next());
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
        auto const meow(__rt_ctx->intern_ns(make_box<runtime::obj::symbol>("meow")));
        __rt_ctx->current_ns()
          ->add_alias(make_box<runtime::obj::symbol>("foo.bar"), meow)
          .expect_ok();
        processor p{ lp.begin(), lp.end() };
        for(auto const &s : { std::make_pair("meow", "bar") })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::symbol>(s.first, s.second)));
        }
      }

      SUBCASE("Qualified, non-existent ns")
      {
        lex::processor lp{ "foo.bar.non-existent/bar" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_err());
      }

      SUBCASE("Quoted")
      {
        lex::processor lp{ "'foo 'bar/spam 'foo.bar/bar" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { std::make_pair("", "foo"),
                              std::make_pair("bar", "spam"),
                              std::make_pair("foo.bar", "bar") })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       make_box<runtime::obj::persistent_list>(
                                         std::in_place,
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "bar", "spam" })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       __rt_ctx->intern_keyword(s).expect_ok()));

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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { std::make_pair("foo", "foo"),
                              std::make_pair("foo.bar", "bar"),
                              std::make_pair("spam.bar", "spam") })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       __rt_ctx->intern_keyword(s.first, s.second).expect_ok()));

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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(auto const &s : { "foo", "spam" })
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       __rt_ctx->intern_keyword("", s, false).expect_ok()));

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
        lex::processor lp{ "::foo.not-aliased/foo" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(r.is_err());
      }

      SUBCASE("Auto-resolved qualified, with alias")
      {
        lex::processor lp{ "::foo/foo" };
        auto const foo_ns(__rt_ctx->intern_ns(make_box<runtime::obj::symbol>("foo.bar.spam")));
        auto const clojure_ns(__rt_ctx->find_ns(make_box<runtime::obj::symbol>("clojure.core")));
        clojure_ns.unwrap()->add_alias(make_box<runtime::obj::symbol>("foo"), foo_ns).expect_ok();
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     __rt_ctx->intern_keyword("foo.bar.spam", "foo").expect_ok()));
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{}; i < 3; ++i)
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{ 1 }; i < 3; ++i)
        {
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_list>(std::in_place,
                                                    make_box<runtime::obj::integer>(1 * i),
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
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(runtime::detail::equal(
          r1.expect_ok().unwrap().ptr,
          make_box<runtime::obj::persistent_list>(std::in_place,
                                                  make_box<runtime::obj::symbol>("def"),
                                                  make_box<runtime::obj::symbol>("foo-bar"),
                                                  make_box<runtime::obj::integer>(1))));
        auto const r2(p.next());
        CHECK(runtime::detail::equal(r2.expect_ok().unwrap().ptr,
                                     make_box<runtime::obj::symbol>("foo-bar")));
      }

      SUBCASE("Extra close")
      {
        lex::processor lp{ "1)" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(runtime::detail::equal(r1.expect_ok().unwrap().ptr, make_box(1)));
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
        lex::processor lp{ "[] [ ] [  ]" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{}; i < 3; ++i)
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{ 1 }; i < 3; ++i)
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(runtime::detail::equal(r1.expect_ok().unwrap().ptr, make_box(1)));
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
        lex::processor lp{ "{} { } {,,}" };
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{}; i < 3; ++i)
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };

        size_t offset{};
        for(size_t i{ 1 }; i < 3; ++i)
        {
          auto const r(p.next());
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
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(
          r.expect_ok().unwrap().ptr,
          make_box<runtime::obj::persistent_array_map>(
            runtime::detail::in_place_unique{},
            make_array_box<runtime::object_ptr>(__rt_ctx->intern_keyword("foo").expect_ok(),
                                                make_box<runtime::obj::boolean>(true),
                                                make_box<runtime::obj::integer>(1),
                                                __rt_ctx->intern_keyword("one").expect_ok(),
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
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("Extra close")
      {
        lex::processor lp{ ":foo}" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(runtime::detail::equal(r1.expect_ok().unwrap().ptr,
                                     __rt_ctx->intern_keyword("foo").expect_ok()));
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

    TEST_CASE("Meta hint")
    {
      SUBCASE("No following meta value")
      {
        lex::processor lp{ "^" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("No following target value")
      {
        lex::processor lp{ "^:foo" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("Keyword meta for a metadatable target")
      {
        lex::processor lp{ "^:foo {}" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     runtime::obj::persistent_array_map::empty()));
        CHECK(runtime::detail::equal(runtime::meta(r.expect_ok().unwrap().ptr),
                                     runtime::obj::persistent_array_map::create_unique(
                                       __rt_ctx->intern_keyword("foo").expect_ok(),
                                       runtime::obj::boolean::true_const())));
      }

      SUBCASE("Keyword meta for non-metadatable target")
      {
        lex::processor lp{ "^:foo nil" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("Map meta for a metadatable target")
      {
        lex::processor lp{ "^{:foo :bar} []" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     runtime::obj::persistent_vector::empty()));
        CHECK(runtime::detail::equal(runtime::meta(r.expect_ok().unwrap().ptr),
                                     runtime::obj::persistent_array_map::create_unique(
                                       __rt_ctx->intern_keyword("foo").expect_ok(),
                                       __rt_ctx->intern_keyword("bar").expect_ok())));
      }

      SUBCASE("Map meta for non-metadatable target")
      {
        lex::processor lp{ "^{:foo :bar} 7.5" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("Multiple meta hints for a metadatable target")
      {
        lex::processor lp{ "^{:foo :bar} ^:meow ()" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     runtime::obj::persistent_list::empty()));
        CHECK(runtime::detail::equal(runtime::meta(r.expect_ok().unwrap().ptr),
                                     runtime::obj::persistent_array_map::create_unique(
                                       __rt_ctx->intern_keyword("foo").expect_ok(),
                                       __rt_ctx->intern_keyword("bar").expect_ok(),
                                       __rt_ctx->intern_keyword("meow").expect_ok(),
                                       runtime::obj::boolean::true_const())));
      }

      SUBCASE("Nested hints")
      {
        lex::processor lp{ "^{:foo ^:meow 'bar} []" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                     runtime::obj::persistent_vector::empty()));
        CHECK(runtime::detail::equal(
          runtime::meta(r.expect_ok().unwrap().ptr),
          runtime::obj::persistent_array_map::create_unique(
            __rt_ctx->intern_keyword("foo").expect_ok(),
            make_box<runtime::obj::persistent_list>(std::in_place,
                                                    make_box<runtime::obj::symbol>("quote"),
                                                    make_box<runtime::obj::symbol>("bar")))));
      }

      SUBCASE("Within a call")
      {
        lex::processor lp{ "(str ^:foo #{})" };
        processor p{ lp.begin(), lp.end() };
        auto const r(p.next());
        CHECK(runtime::detail::equal(
          r.expect_ok().unwrap().ptr,
          make_box<runtime::obj::persistent_list>(std::in_place,
                                                  make_box<runtime::obj::symbol>("str"),
                                                  runtime::obj::persistent_set::empty())));
        CHECK(runtime::detail::equal(
          runtime::meta(
            runtime::expect_object<runtime::obj::persistent_list>(r.expect_ok().unwrap().ptr)
              ->data.rest()
              .first()
              .unwrap()),
          runtime::obj::persistent_array_map::create_unique(
            __rt_ctx->intern_keyword("foo").expect_ok(),
            runtime::obj::boolean::true_const())));
      }
    }

    TEST_CASE("Reader macro")
    {
      SUBCASE("No following value")
      {
        lex::processor lp{ "#" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("Unsupported following value")
      {
        lex::processor lp{ "#[]" };
        processor p{ lp.begin(), lp.end() };
        auto const r1(p.next());
        CHECK(r1.is_err());
      }

      SUBCASE("Deref")
      {
        SUBCASE("Unterminated")
        {
          lex::processor lp{ "@" };
          processor p{ lp.begin(), lp.end() };
          auto const r1(p.next());
          CHECK(r1.is_err());
        }

        SUBCASE("Single")
        {
          lex::processor lp{ "@foo" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_list>(std::in_place,
                                                    make_box<runtime::obj::symbol>("deref"),
                                                    make_box<runtime::obj::symbol>("foo"))));
        }

        SUBCASE("Double")
        {
          lex::processor lp{ "@@foo" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_list>(
              std::in_place,
              make_box<runtime::obj::symbol>("deref"),
              make_box<runtime::obj::persistent_list>(std::in_place,
                                                      make_box<runtime::obj::symbol>("deref"),
                                                      make_box<runtime::obj::symbol>("foo")))));
        }
      }

      SUBCASE("Set")
      {
        SUBCASE("Empty")
        {
          lex::processor lp{ "#{}" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                       runtime::obj::persistent_set::empty()));
        }

        SUBCASE("Non-empty")
        {
          lex::processor lp{ "#{1}" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_set>(std::in_place, make_box(1))));
        }

        SUBCASE("Nested")
        {
          lex::processor lp{ "#{1, #{2}}" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_set>(
              std::in_place,
              make_box(1),
              make_box<runtime::obj::persistent_set>(std::in_place, make_box(2)))));
        }
      }

      SUBCASE("Comment")
      {
        SUBCASE("EOF")
        {
          lex::processor lp{ "#_" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(r.is_err());
        }

        SUBCASE("Other reader macro")
        {
          lex::processor lp{ "#_#{1} #{2}" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_set>(std::in_place, make_box(2))));
        }

        SUBCASE("Adjacent")
        {
          lex::processor lp{ "#_#_1 2 3" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box(3)));
        }

        SUBCASE("Number")
        {
          lex::processor lp{ "#_1.23 \"ok\"" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr, make_box("ok")));
        }

        SUBCASE("Invalid form")
        {
          lex::processor lp{ "#_{1.23} \"not ok\"" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(r.is_err());
        }
      }

      SUBCASE("Conditional")
      {
        SUBCASE("EOF")
        {
          lex::processor lp{ "#?" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(r.is_err());
        }

        SUBCASE("Non-list after")
        {
          lex::processor lp{ "#?[]" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(r.is_err());
        }

        SUBCASE("No match")
        {
          lex::processor lp{ "[#?(:clj 0 :cljs 1) 9]" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_vector>(std::in_place, make_box(9))));
        }

        SUBCASE("Default match")
        {
          lex::processor lp{ "[#?(:clj 0 :cljs 1 :default 8) 9]" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_vector>(std::in_place, make_box(8), make_box(9))));
        }

        SUBCASE("jank match")
        {
          lex::processor lp{ "[#?(:clj 0 :cljs 1 :jank 7 :default 8) 9]" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_vector>(std::in_place, make_box(7), make_box(9))));
        }

        SUBCASE("First match picked")
        {
          lex::processor lp{ "[#?(:default -1 :clj 0 :cljs 1 :jank 7 :default 8) 9]" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_vector>(std::in_place, make_box(-1), make_box(9))));
        }

        SUBCASE("Nested")
        {
          lex::processor lp{ "[#?(:clj 0 :cljs 1 :jank #?(:default 5) :default 8) 9]" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_vector>(std::in_place, make_box(5), make_box(9))));
        }

        SUBCASE("Other reader macro")
        {
          lex::processor lp{ "#?(:default #{1})" };
          processor p{ lp.begin(), lp.end() };
          auto const r(p.next());
          CHECK(runtime::detail::equal(
            r.expect_ok().unwrap().ptr,
            make_box<runtime::obj::persistent_set>(std::in_place, make_box(1))));
        }

        SUBCASE("Splice")
        {
          SUBCASE("Not seqable")
          {
            lex::processor lp{ "#?@(:jank 2)" };
            processor p{ lp.begin(), lp.end() };
            auto const r(p.next());
            CHECK(r.is_err());
          }

          SUBCASE("Top-level, empty")
          {
            lex::processor lp{ "#?@(:jank [])" };
            processor p{ lp.begin(), lp.end() };
            auto const r(p.next());
            CHECK(r.is_err());
          }

          SUBCASE("Top-level, non-empty")
          {
            lex::processor lp{ "#?@(:jank [1])" };
            processor p{ lp.begin(), lp.end() };
            auto const r(p.next());
            CHECK(r.is_err());
          }

          /* NOTE: Clojure doesn't allow this, but we do. At least until jank has a better
           * abstraction for knowing which sequences are ordered. */
          SUBCASE("Unordered sequence")
          {
            lex::processor lp{ "(#?@(:jank #{1 2}))" };
            processor p{ lp.begin(), lp.end() };
            auto const r(p.next());
            CHECK(runtime::detail::equal(
              r.expect_ok().unwrap().ptr,
              make_box<runtime::obj::persistent_list>(std::in_place, make_box(1), make_box(2))));
          }

          SUBCASE("Nested")
          {
            lex::processor lp{ "[#?@(:jank [1 #?@(:clj [1.2 1.3]) 2 #?@(:default [3 4]) 5 6])]" };
            processor p{ lp.begin(), lp.end() };
            auto const r(p.next());
            CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                         make_box<runtime::obj::persistent_vector>(std::in_place,
                                                                                   make_box(1),
                                                                                   make_box(2),
                                                                                   make_box(3),
                                                                                   make_box(4),
                                                                                   make_box(5),
                                                                                   make_box(6))));
          }

          SUBCASE("Within a set, non-empty splice")
          {
            lex::processor lp{ "#{0 #?@(:default [1]) 2}" };
            processor p{ lp.begin(), lp.end() };
            auto const r(p.next());
            CHECK(runtime::detail::equal(r.expect_ok().unwrap().ptr,
                                         make_box<runtime::obj::persistent_set>(std::in_place,
                                                                                make_box(0),
                                                                                make_box(1),
                                                                                make_box(2))));
          }
        }
      }
    }
  }
}
