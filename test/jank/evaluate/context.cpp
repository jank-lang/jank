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
  TEST_CASE("Call" * doctest::may_fail())
  {
    runtime::context rt_ctx;
    analyze::context an_ctx{ rt_ctx };
    auto const result(rt_ctx.eval_string("(+ 777 2)", an_ctx));
    CHECK(result != nullptr);
    CHECK(result->equal(runtime::obj::integer{ 779 }));
  }

  TEST_CASE("Function")
  {
    runtime::context rt_ctx;
    analyze::context an_ctx{ rt_ctx };

    SUBCASE("Nullary")
    {
      auto const result(rt_ctx.eval_string("(fn* [] 1)", an_ctx));
      CHECK(result != nullptr);
      CHECK(result->as_function() != nullptr);
      CHECK(result->as_function()->call()->equal(runtime::obj::integer{ 1 }));
    }

    SUBCASE("Unary")
    {
      auto const result(rt_ctx.eval_string("(fn* [a] a)", an_ctx));
      CHECK(result != nullptr);
      CHECK(result->as_function() != nullptr);
      CHECK(result->as_function()->call(runtime::make_box<runtime::obj::integer>(1))->equal(runtime::obj::integer{ 1 }));
    }

    SUBCASE("Binary")
    {
      auto const result(rt_ctx.eval_string("(fn* [a b] [a b])", an_ctx));
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
    analyze::context an_ctx{ rt_ctx };

    SUBCASE("Empty list")
    {
      auto const result(rt_ctx.eval_string("()", an_ctx));
      CHECK(result != nullptr);
      CHECK(result->equal(runtime::obj::list{ }));
    }

    SUBCASE("Quoted")
    {
      SUBCASE("List")
      {
        auto const result(rt_ctx.eval_string("'(1 2 (foo))", an_ctx));
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
        auto const result(rt_ctx.eval_string("'[1 2 (foo)]", an_ctx));
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
        auto const result(rt_ctx.eval_string("'foo/bar", an_ctx));
        CHECK(result != nullptr);
        CHECK(result->equal(runtime::obj::symbol{ "foo/bar" }));
      }

      SUBCASE("Integer")
      {
        auto const result(rt_ctx.eval_string("'123", an_ctx));
        CHECK(result != nullptr);
        CHECK(result->equal(runtime::obj::integer{ 123 }));
      }
    }
  }

  TEST_CASE("Var")
  {
    runtime::context rt_ctx;
    analyze::context an_ctx{ rt_ctx };

    SUBCASE("Basic")
    {
      auto const result(rt_ctx.eval_string("(def foo-bar 1) foo-bar", an_ctx));
      CHECK(result != nullptr);
      CHECK(result->as_integer() != nullptr);
      CHECK(result->as_integer()->equal(runtime::obj::integer{ 1 }));
    }

    SUBCASE("Redefinition")
    {
      auto const result(rt_ctx.eval_string("(def foo-bar 1) (def foo-bar 'meow) foo-bar", an_ctx));
      CHECK(result != nullptr);
      CHECK(result->as_symbol() != nullptr);
      CHECK(result->equal(runtime::obj::symbol{ "meow" }));
    }
  }
}
