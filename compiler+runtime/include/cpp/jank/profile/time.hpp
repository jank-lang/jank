#pragma once

#include <jank/util/cli.hpp>

namespace jank::profile
{
  void configure(util::cli::options const &opts);
  bool is_enabled();
  void enter(native_persistent_string_view const &region);
  void exit(native_persistent_string_view const &region);
  void report(native_persistent_string_view const &boundary);

  struct timer
  {
    timer() = delete;
    timer(native_persistent_string_view const &region);
    ~timer();

    void report(native_persistent_string_view const &boundary) const;

    jtl::immutable_string region;
  };
}
