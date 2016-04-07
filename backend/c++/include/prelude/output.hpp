#pragma once

#include <iostream>
#include <sstream>

#include "primitive.hpp"

namespace jank
{
  template <typename T>
  string print(T const &t)
  {
    std::stringstream ss;
    ss << t;
    std::cout << ss.rdbuf() << std::endl;
    return ss.str();
  }
}
