#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::ir
{
  struct module;
}

namespace jank::codegen
{
  struct generated_cpp
  {
    jtl::immutable_string declaration;
    jtl::immutable_string expresssion;
  };

  generated_cpp gen_cpp(ir::module const &mod);
}
