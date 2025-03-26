#pragma once

#include <filesystem>
#include <memory>

#include <clang/Interpreter/Interpreter.h>

#include <jtl/result.hpp>
#include <jank/util/cli.hpp>
#include <jank/util/string_builder.hpp>

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
    processor(util::cli::options const &opts);
    ~processor();

    void eval_string(native_persistent_string const &s) const;
    void load_object(native_persistent_string_view const &path) const;
    void load_dynamic_library(native_persistent_string const &path) const;
    void load_ir_module(std::unique_ptr<llvm::Module> m,
                        std::unique_ptr<llvm::LLVMContext> llvm_ctx) const;
    void load_bitcode(native_persistent_string const &module,
                      native_persistent_string_view const &bitcode) const;

    jtl::string_result<void> remove_symbol(native_persistent_string const &name) const;

    template <typename T>
    jtl::string_result<T> find_symbol(native_persistent_string const &name) const
    {
      if(auto symbol{ interpreter->getSymbolAddress(name.c_str()) })
      {
        return symbol.get().toPtr<T>();
      }

      util::string_builder sb;
      sb("Failed for find symbol: '")(name)("'");
      return err(sb.release());
    }

    jtl::result<void, native_persistent_string>
    load_dynamic_libs(native_vector<native_persistent_string> const &libs) const;
    jtl::option<native_persistent_string> find_dynamic_lib(native_persistent_string const &lib) const;

    std::unique_ptr<clang::Interpreter> interpreter;
    native_integer optimization_level{};
    native_vector<std::filesystem::path> library_dirs;
  };
}
