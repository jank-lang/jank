#pragma once

#include <filesystem>
#include <map>

#include <jtl/result.hpp>
#include <jtl/string_builder.hpp>

#include <jank/runtime/object.hpp>

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

namespace CppInternal
{
  class Interpreter;
}

namespace jank::ir
{
  struct module;
}

namespace jank::runtime::obj
{
  using jit_function_ref = oref<struct jit_function>;
}

namespace jank::jit
{
  struct resolved_lib
  {
    /* The resolved lib name/path. */
    jtl::immutable_string lib;
    /* Whether or not the resolved lib is a static lib. */
    bool is_static{};
  };

  struct processor
  {
    processor(jtl::immutable_string const &binary_version);
    ~processor();

    runtime::object_ref eval(ir::module const &module) const;
    runtime::object_ref create_function(runtime::callable_arity_flags flags,
                                        jtl::immutable_string const &base_name,
                                        native_vector<u8> const &arities,
                                        bool const is_variadic) const;

    void eval_string(jtl::immutable_string const &s) const;
    void eval_string(jtl::immutable_string const &s, clang::Value *) const;
    void load_object(jtl::immutable_string_view const &path) const;
    void load_dynamic_library(jtl::immutable_string const &path) const;
    void load_static_library(jtl::immutable_string const &path) const;
    void load_ir_module(llvm::orc::ThreadSafeModule &&m) const;
    void load_bitcode(jtl::immutable_string const &module,
                      jtl::immutable_string_view const &bitcode) const;

    jtl::string_result<void> remove_symbol(jtl::immutable_string const &name) const;
    jtl::string_result<void *> find_symbol(jtl::immutable_string const &name) const;

    static native_vector<std::filesystem::path> build_library_dirs();
    static jtl::result<native_vector<resolved_lib>, jtl::immutable_string>
    resolve_libs(native_vector<jtl::immutable_string> const &libs);
    jtl::result<void, jtl::immutable_string>
    load_libs(native_vector<jtl::immutable_string> const &libs) const;
    static jtl::option<jtl::immutable_string>
    find_lib(native_vector<std::filesystem::path> const &library_dirs,
             jtl::immutable_string const &lib);

    /*** XXX: Everything here is immutable after initialization. ***/
    /*** XXX: Calls through the interpreter and LLVM JIT runtime are thread-safe. ***/
    native_vector<std::filesystem::path> library_dirs;

    /* The files within this map will get added into Clang's VFS prior to the creation of
     * the `clang::Interpreter`. This allows us to embed the PCH into AOT compiled programs
     * while still being able to include it. */
    std::map<char const *, std::string_view> vfs;

    /*** XXX: Everything here is thread-safe. ***/
    jtl::ptr<CppInternal::Interpreter> interpreter;
  };
}
