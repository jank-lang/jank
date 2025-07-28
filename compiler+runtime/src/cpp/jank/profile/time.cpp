#include <fstream>

#include <jank/profile/time.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/cli.hpp>

namespace jank::profile
{
  using util::cli::opts;

  static constexpr jtl::immutable_string_view tag{ "jank::profile" };
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static std::ofstream output;

  static auto now()
  {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
  }

  void configure()
  {
    if(opts.profiler_enabled)
    {
      output.open(opts.profiler_file.data());
      if(!output.is_open())
      {
        opts.profiler_enabled = false;
        util::println(stderr,
                      "Unable to open profile file: {}\nProfiling is now disabled.",
                      opts.profiler_file);
      }
    }
  }

  bool is_enabled()
  {
    return opts.profiler_enabled;
  }

  void enter(jtl::immutable_string_view const &region)
  {
    if(opts.profiler_enabled)
    {
      output << util::format("{} {} enter {}\n", tag, now(), region);
    }
  }

  void exit(jtl::immutable_string_view const &region)
  {
    if(opts.profiler_enabled)
    {
      output << util::format("{} {} exit {}\n", tag, now(), region);
    }
  }

  void report(jtl::immutable_string_view const &boundary)
  {
    if(opts.profiler_enabled)
    {
      output << util::format("{} {} report {}\n", tag, now(), boundary);
    }
  }

  timer::timer(jtl::immutable_string_view const &region)
    : region{ region }
  {
    enter(region);
  }

  timer::~timer()
  try
  {
    exit(region);
  }
  catch(...)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): I need to log without exceptions. */
    std::printf("Exception caught while destructing timer");
  }

  void timer::report(jtl::immutable_string_view const &boundary) const
  {
    jank::profile::report(boundary);
  }
}
