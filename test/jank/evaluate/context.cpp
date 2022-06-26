#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/evaluate/context.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

namespace jank::evaluate
{
  TEST_CASE("Call")
  {
    read::lex::processor l_prc{ "(+ 777 2)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    analyze::processor anal_prc{ rt_ctx };
    context eval_ctx{ rt_ctx };

    auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
    CHECK(result != nullptr);
    CHECK(result->equal(runtime::obj::integer{ 779 }));
  }

  TEST_CASE("Function")
  {
    runtime::context rt_ctx;
    analyze::processor anal_prc{ rt_ctx };
    context eval_ctx{ rt_ctx };

    SUBCASE("Nullary")
    {
      read::lex::processor l_prc{ "(fn* [] 1)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
      CHECK(result != nullptr);
      CHECK(result->as_function() != nullptr);
      CHECK(result->as_function()->call()->equal(runtime::obj::integer{ 1 }));
    }

    SUBCASE("Unary")
    {
      read::lex::processor l_prc{ "(fn* [a] a)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
      CHECK(result != nullptr);
      CHECK(result->as_function() != nullptr);
      CHECK(result->as_function()->call(runtime::make_box<runtime::obj::integer>(1))->equal(runtime::obj::integer{ 1 }));
    }

    SUBCASE("Binary")
    {
      read::lex::processor l_prc{ "(fn* [a b] [a b])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
      CHECK(result != nullptr);
      CHECK(result->as_function() != nullptr);

      auto const a0(runtime::make_box<runtime::obj::integer>(1));
      auto const a1(runtime::make_box<runtime::obj::string>("meow"));
      CHECK(result->as_function()->call(a0, a1)->equal(runtime::obj::vector::create(a0, a1)));
    }
  }

  TEST_CASE("Literals")
  {
    runtime::context rt_ctx;
    analyze::processor anal_prc{ rt_ctx };
    context eval_ctx{ rt_ctx };

    SUBCASE("Empty list")
    {
      read::lex::processor l_prc{ "()" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
      CHECK(result != nullptr);
      CHECK(result->equal(runtime::obj::list{ }));
    }

    SUBCASE("Quoted")
    {
      SUBCASE("List")
      {
        read::lex::processor l_prc{ "'(1 2 (foo))" };
        read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

        auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
        CHECK(result != nullptr);
        CHECK
        (
          result->equal
          (
            runtime::obj::list
            {
              runtime::make_box<runtime::obj::integer>(1),
              runtime::make_box<runtime::obj::integer>(2),
              runtime::make_box<runtime::obj::list>(runtime::obj::symbol::create("foo"))
            }
          )
        );
      }

      SUBCASE("Vector")
      {
        read::lex::processor l_prc{ "'[1 2 (foo)]" };
        read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

        auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
        CHECK(result != nullptr);
        CHECK
        (
          result->equal
          (
            runtime::obj::vector
            {
              runtime::make_box<runtime::obj::integer>(1),
              runtime::make_box<runtime::obj::integer>(2),
              runtime::make_box<runtime::obj::list>(runtime::obj::symbol::create("foo"))
            }
          )
        );
      }

      SUBCASE("Symbol")
      {
        read::lex::processor l_prc{ "'foo/bar" };
        read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

        auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
        CHECK(result != nullptr);
        CHECK(result->equal(runtime::obj::symbol{ "foo/bar" }));
      }

      SUBCASE("Integer")
      {
        read::lex::processor l_prc{ "'123" };
        read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

        auto const result(eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok())));
        CHECK(result != nullptr);
        CHECK(result->equal(runtime::obj::integer{ 123 }));
      }
    }
  }
}
