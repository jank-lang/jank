#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::analyze::expr
{
  TEST_CASE("No arg")
  {
    read::lex::processor l_prc{ "(native/raw)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Invalid arg type")
  {
    read::lex::processor l_prc{ "(native/raw wow-much-raw)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Empty string")
  {
    read::lex::processor l_prc{ R"((native/raw ""))" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *native_raw_expr(boost::get<native_raw<expression>>(&expr.data));
    CHECK(native_raw_expr != nullptr);
    CHECK(native_raw_expr->chunks.empty());
  }

  TEST_CASE("C++ code")
  {
    read::lex::processor l_prc{ R"((native/raw "std::cout << 77;"))" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *native_raw_expr(boost::get<native_raw<expression>>(&expr.data));
    CHECK(native_raw_expr != nullptr);
    CHECK(native_raw_expr->chunks.size() == 1);
    CHECK(boost::get<runtime::detail::string_type>(native_raw_expr->chunks[0]) == "std::cout << 77;");
  }

  TEST_CASE("Interpolation")
  {
    SUBCASE("Valid")
    {
      read::lex::processor l_prc{ R"((def a 1) (native/raw "std::cout << #{a}# << std::endl;"))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      runtime::context rt_ctx;
      context an_ctx{ rt_ctx };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
      auto const expr(*(++boost::get<function<expression>>(fn_expr.data).body.body.begin()));
      auto const *native_raw_expr(boost::get<native_raw<expression>>(&expr.data));
      CHECK(native_raw_expr != nullptr);
      CHECK(native_raw_expr->chunks.size() == 3);
      CHECK(boost::get<runtime::detail::string_type>(native_raw_expr->chunks[0]) == "std::cout << ");
      CHECK
      (
        boost::get<analyze::expr::var_deref<analyze::expression>>
        (
          boost::get<analyze::expression>(native_raw_expr->chunks[1]).data
        ).qualified_name->equal(runtime::obj::symbol{ "clojure.core", "a" })
      );
      CHECK(boost::get<runtime::detail::string_type>(native_raw_expr->chunks[2]) == " << std::endl;");
    }

    SUBCASE("Invalid interpolation code")
    {
      read::lex::processor l_prc{ R"((native/raw "#{ does-not-exist }#"))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      runtime::context rt_ctx;
      context an_ctx{ rt_ctx };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx));
      CHECK(fn_expr.is_err());
    }

    SUBCASE("Too many expressions")
    {
      read::lex::processor l_prc{ R"((native/raw "#{ 1 2 }#"))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      runtime::context rt_ctx;
      context an_ctx{ rt_ctx };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx));
      CHECK(fn_expr.is_err());
    }
  }
}
