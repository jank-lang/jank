#include <nanobench.h>

#include <jank/runtime/perf.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::perf
{
  object_ref benchmark(object_ref const opts, object_ref const f)
  {
    auto const label(get(opts, __rt_ctx->intern_keyword("label").expect_ok()));
    auto const label_str(to_string(label));
    ankerl::nanobench::Config config;
    config.mOut = &std::cout;

    /* Larger things. */
    config.mTimeUnitName = "ms";
    config.mTimeUnit = std::chrono::milliseconds{ 1 };
    config.mMinEpochIterations = 20;
    config.mWarmup = 10;

    /* Smaller things. */
    //config.mTimeUnitName = "ns";
    //config.mTimeUnit = std::chrono::nanoseconds{ 1 };
    //config.mMinEpochIterations = 1000000;
    //config.mWarmup = 1000;

    ankerl::nanobench::Bench().config(config).run(static_cast<std::string>(label_str), [&] {
      auto const res(f->call());
      ankerl::nanobench::doNotOptimizeAway(res);
    });
    return {};
  }
}
