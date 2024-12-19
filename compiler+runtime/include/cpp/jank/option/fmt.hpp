#pragma once

#include <ostream>

#include <fmt/ostream.h>

#include <jank/option.hpp>

namespace jank
{
  template <typename T>
  std::ostream &operator<<(std::ostream &os, option<T> const &o)
  {
    if(o.is_none())
    {
      return os << "none";
    }
    return os << "some(" << o.unwrap() << ")";
  }
}

namespace fmt
{
  template <typename T>
  struct formatter<jank::option<T>> : fmt::ostream_formatter
  {
  };
}
