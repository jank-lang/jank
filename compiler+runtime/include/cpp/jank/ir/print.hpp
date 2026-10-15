#pragma once

#include <jank/ir/processor.hpp>

namespace jank::ir
{
  [[nodiscard]]
  jtl::immutable_string print(function const &fn);
  [[nodiscard]]
  jtl::immutable_string print(module const &mod);
}
