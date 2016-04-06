#pragma once

#include <iostream>
#include <sstream>

#include "primitive.hpp"

namespace jank
{
  string input()
  {
    string s;
    std::getline(std::cin, s);
    return s;
  }
}

namespace jank_gen
{
  string input_gen_bang_gen_minus639422135()
  { return jank::input(); }
}
