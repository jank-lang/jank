#pragma once

#include <filesystem>
#include <memory>

#include <jtl/result.hpp>
#include <jtl/string_builder.hpp>

namespace llvm
{
  class Module;
  class LLVMContext;

  namespace orc
  {
    class ThreadSafeModule;
  }
}

namespace clang
{
  class Value;
}

namespace Cpp
{
  class Interpreter;
}

namespace jank::jit
{
  struct processor
  {
    processor(jtl::immutable_string const &binary_version);
    ~processor();

    void eval_string(jtl::immutable_string const &s) const;
    void eval_string(jtl::immutable_string const &s, clang::Value *) const;
    void load_object(jtl::immutable_string_view const &path) const;
    void load_dynamic_library(jtl::immutable_string const &path) const;
    void load_ir_module(llvm::orc::ThreadSafeModule &&m) const;
    void load_bitcode(jtl::immutable_string const &module,
                      jtl::immutable_string_view const &bitcode) const;

    jtl::string_result<void> remove_symbol(jtl::immutable_string const &name) const;
    jtl::string_result<void *> find_symbol(jtl::immutable_string const &name) const;

    jtl::result<void, jtl::immutable_string>
    load_dynamic_libs(native_vector<jtl::immutable_string> const &libs) const;
    jtl::option<jtl::immutable_string> find_dynamic_lib(jtl::immutable_string const &lib) const;

    std::unique_ptr<Cpp::Interpreter> interpreter;
    native_vector<std::filesystem::path> library_dirs;

    /* The files within this map will get added into Clang's VFS prior to the creation of
     * the `clang::Interpreter`. This allows us to embed the PCH into AOT compiled programs
     * while still being able to include it. */
    std::map<char const *, std::string_view> vfs;
  };
}
