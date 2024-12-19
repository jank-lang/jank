#pragma once

#include <fmt/ostream.h>

#include <jank/runtime/native_box.hpp>

namespace fmt
{
  template <typename T>
  struct formatter<jank::runtime::native_box<T>> : fmt::ostream_formatter
  {
  };
}
