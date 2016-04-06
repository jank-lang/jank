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

namespace jank_gen
{
  string print_gen_bang1925837317(string s)
  { return jank::print(s); }
}
