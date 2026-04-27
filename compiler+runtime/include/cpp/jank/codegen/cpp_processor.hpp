#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::ir
{
  struct module;
}

namespace jank::codegen
{
  /* We keep track of our compilation target so we can perform certain optimizations. For example,
   * when we're compiling for `eval`, we know that the generated code will only be used in the
   * current process. So, instead of defining our own lifted constants and vars, we generate in
   * actual memory addresses for where those objects already live in the runtime.
   *
   * On the other hand, during module compilation, we combine everything into one namespace, share
   * constants and vars across all module functions, and we have a special `jank_load_foo` function
   * which helps initialize the module for the jank runtime. This is the function we look for
   * when we load this module from an object file. */
  enum class compilation_target : u8
  {
    module,
    /* A module function is a nested function within an outer module. It will share the same state
     * as the outer module and will not be standalone. */
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
    /* This is the generated C++ to declare and define all functions and values within
     * an IR module. */
    jtl::immutable_string declaration;
    /* This is the generated C++ to create an instance of the primary function for this module. */
    jtl::immutable_string expression;
  };

  generated_cpp gen_cpp(ir::module const &mod);
}
