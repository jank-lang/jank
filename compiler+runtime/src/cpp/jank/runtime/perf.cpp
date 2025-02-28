#include <nanobench.h>

#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/perf.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::perf
{
  object_ptr benchmark(object_ptr const opts, object_ptr const f)
  {
    auto const label(get(opts, __rt_ctx->intern_keyword("label").expect_ok()));
    auto const label_str(to_string(label));
    visit_object(
      [](auto const typed_f, native_persistent_string const &label) {
        using T = typename decltype(typed_f)::value_type;

        if constexpr(std::is_base_of_v<behavior::callable, T>)
        {
          ankerl::nanobench::Config config;
          //config.mTimeUnitName = TODO
          config.mOut = &std::cout;

          /* Larger things. */
          config.mTimeUnit = std::chrono::milliseconds{ 1 };
          config.mMinEpochIterations = 20;
          config.mWarmup = 10;

          /* Smaller things. */
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
          throw std::runtime_error{ fmt::format("not callable: {}", typed_f->to_string()) };
        }
      },
      f,
      label_str);
    return obj::nil::nil_const();
  }
}
