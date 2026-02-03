#pragma once

#include <jtl/ref.hpp>

namespace jank::codegen
{
  void optimize(jtl::ref<llvm::Module> const module, jtl::immutable_string const &module_name);
}
