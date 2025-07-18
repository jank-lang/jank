#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/option.hpp>
#include <jtl/result.hpp>

namespace jank::util
{
  jtl::option<jtl::immutable_string> find_clang();
  jtl::result<void, jtl::immutable_string> invoke_clang(std::vector<char const *> const &args);
}
