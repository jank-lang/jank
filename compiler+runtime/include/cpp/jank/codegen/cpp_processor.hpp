#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::ir
{
  struct module;
}

namespace jank::codegen
{
  enum class compilation_target : u8
  {
    module,
    module_function,
    eval
  };

  constexpr char const *compilation_target_str(compilation_target const t)
  {
    switch(t)
    {
      case compilation_target::module:
        return "module";
      case compilation_target::module_function:
        return "module_function";
      case compilation_target::eval:
        return "eval";
      default:
        return "unknown";
    }
  }

  struct generated_cpp
  {
    jtl::immutable_string declaration;
    jtl::immutable_string expresssion;
  };

  generated_cpp gen_cpp(ir::module const &mod);
}
