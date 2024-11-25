#pragma once

#include <memory>

#include <clang/Interpreter/Interpreter.h>

#include <jank/result.hpp>

namespace llvm
{
  class Module;
  class LLVMContext;
}

namespace clang
{
  class Interpreter;
}

namespace jank::jit
{
  struct processor
  {
    processor(native_integer optimization_level);
    ~processor();

    void eval_string(native_persistent_string const &s) const;
    void load_object(native_persistent_string_view const &path) const;
    void load_ir_module(std::unique_ptr<llvm::Module> m,
                        std::unique_ptr<llvm::LLVMContext> llvm_ctx) const;
    void load_bitcode(native_persistent_string const &module,
                      native_persistent_string const &bitcode) const;

    template <typename T>
    T find_symbol(native_persistent_string const &name) const
    {
      auto const sym(interpreter->getSymbolAddress(name.c_str()).get());
      return sym.toPtr<T>();
    }

    std::unique_ptr<clang::Interpreter> interpreter;
    native_integer optimization_level{};
  };
}
