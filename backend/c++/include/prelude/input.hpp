#pragma once

#include <iostream>
#include <sstream>

#include "primitive.hpp"

namespace jank
{
  string input_gen_bang()
  {
    string s;
    std::getline(std::cin, s);
    return s;
  }
}
