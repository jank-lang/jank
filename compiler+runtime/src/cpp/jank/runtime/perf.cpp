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
    visit_object(
      [](auto const typed_f, jtl::immutable_string const &label) {
        using T = typename decltype(typed_f)::value_type;

        if constexpr(std::is_base_of_v<behavior::callable, T>)
        {
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

          ankerl::nanobench::Bench().config(config).run(static_cast<std::string>(label), [&] {
            auto const res(typed_f->call());
            ankerl::nanobench::doNotOptimizeAway(res);
          });
        }
        else
        {
          throw std::runtime_error{ util::format("not callable: {}", typed_f->to_string()) };
        }
      },
      f,
      label_str);
    return jank_nil;
  }
}
