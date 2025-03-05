#pragma once

#include <string>

namespace jank::util
{
  std::string to_lowercase(std::string const &s);
  std::string to_uppercase(std::string const &s);
  void trim(std::string &s);
}
