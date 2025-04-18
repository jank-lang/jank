#pragma once

#include <string>

#include <jtl/primitive.hpp>

namespace jank::util
{
  std::string to_lowercase(std::string const &s);
  std::string to_uppercase(std::string const &s);
  void trim(std::string &s);
  void capitalize(std::string &s);
  std::string ordinal_under_100(usize n);
  std::string number_to_ordinal(usize n);
}
