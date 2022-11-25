#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::analyze::expr
{
  TEST_CASE("Empty")
  {
    read::lex::processor l_prc{ "(def f (fn* [])) (f 1)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }
}
