#pragma once

#include <filesystem>
#include <memory>

#include <jtl/result.hpp>
#include <jank/util/cli.hpp>
#include <jank/util/string_builder.hpp>

namespace llvm
{
  class Module;
  class LLVMContext;
}

namespace Cpp
{
  class Interpreter;
}

namespace jank::jit
{

  struct processor
  {
    processor(util::cli::options const &opts);
    ~processor();

    void eval_string(jtl::immutable_string const &s) const;
    void load_object(native_persistent_string_view const &path) const;
    void load_dynamic_library(jtl::immutable_string const &path) const;
    void load_ir_module(std::unique_ptr<llvm::Module> m,
                        std::unique_ptr<llvm::LLVMContext> llvm_ctx) const;
    void load_bitcode(jtl::immutable_string const &module,
                      native_persistent_string_view const &bitcode) const;

    jtl::string_result<void> remove_symbol(jtl::immutable_string const &name) const;
    jtl::string_result<void *> find_symbol(jtl::immutable_string const &name) const;

    jtl::result<void, jtl::immutable_string>
    load_dynamic_libs(native_vector<jtl::immutable_string> const &libs) const;
    jtl::option<jtl::immutable_string> find_dynamic_lib(jtl::immutable_string const &lib) const;

    std::unique_ptr<Cpp::Interpreter> interpreter;
    native_integer optimization_level{};
    native_vector<std::filesystem::path> library_dirs;
  };
}
