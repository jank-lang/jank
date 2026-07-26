#pragma once

#include <string>

#include <jtl/primitive.hpp>

namespace jtl
{
  struct immutable_string;
}

namespace jank::util
{
  jtl::immutable_string to_lowercase(jtl::immutable_string const &s);
  jtl::immutable_string to_uppercase(jtl::immutable_string const &s);
  void trim(std::string &s);
  std::string trim(jtl::immutable_string const &s);
  jtl::immutable_string capitalize(jtl::immutable_string const &s);
  std::string ordinal_under_100(usize n);
  std::string number_to_ordinal(usize n);
}
