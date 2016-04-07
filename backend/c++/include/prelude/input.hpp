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
