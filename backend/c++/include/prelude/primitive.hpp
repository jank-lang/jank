#pragma once

#include <string>
#include <functional>

namespace jank
{
  using integer = int64_t;
  using real = double;
  using boolean = bool;
  using string = std::string;
  template <typename T>
  using function = std::function<T>;
}
