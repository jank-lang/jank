#pragma once

#include <jank/util/cli.hpp>

namespace jank::profile
{
  void configure(util::cli::options const &opts);
  void enter(native_string_view const &region);
  void exit(native_string_view const &region);
  void report(native_string_view const &boundary);

  struct timer
  {
    timer() = delete;
    timer(native_string_view const &region);
    ~timer();

    void report(native_string_view const &boundary) const;

    native_string region;
  };
}
