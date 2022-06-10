#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/evaluate/context.hpp>

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
}
