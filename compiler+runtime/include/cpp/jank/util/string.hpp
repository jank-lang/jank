#pragma once

#include <string>

namespace jank::util
{
  std::string to_lowercase(std::string const &s);
  std::string to_uppercase(std::string const &s);
  void trim(std::string &s);
  void capitalize(std::string &s);
  std::string ordinal_under_100(size_t n);
  std::string number_to_ordinal(size_t n);
}
