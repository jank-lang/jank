#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include <folly/Synchronized.h>

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

    /* Describes one lazily materialized object-file-backed frame after we've mapped a runtime
     * PC back to its original `.o` file location. The formatter uses this to ask cpptrace to do
     * ordinary on-disk DWARF resolution against the backing object file. */
    struct materialized_object_frame
    {
      uptr runtime_address{};
      usize runtime_size{};
      std::string object_path;
      uptr object_address{};
      usize object_size{};
      std::string symbol;
    };

    jtl::option<materialized_object_frame> lookup_materialized_object_frame(uptr raw_address) const;

    jtl::result<void, jtl::immutable_string>
    load_libs(native_vector<jtl::immutable_string> const &libs) const;
    jtl::option<jtl::immutable_string> find_lib(jtl::immutable_string const &lib) const;

    /*** XXX: Everything here is immutable after initialization. ***/
    /*** XXX: Calls through the interpreter and LLVM JIT runtime are thread-safe. ***/
    native_vector<std::filesystem::path> library_dirs;

    /* The files within this map will get added into Clang's VFS prior to the creation of
     * the `clang::Interpreter`. This allows us to embed the PCH into AOT compiled programs
     * while still being able to include it. */
    std::map<char const *, std::string_view> vfs;

    /*** XXX: Everything here is thread-safe. ***/
    jtl::ptr<CppInternal::Interpreter> interpreter;

    /* Symbol metadata recorded from the original object file before ORC materializes anything. */
    struct loaded_object_symbol
    {
      uptr object_address{};
      usize object_size{};
    };

    /* Bookkeeping for a single object file added through `load_object`. This is keyed by the
     * ORC resource key so we can join it later with the symbols that JITLink actually emits. */
    struct loaded_object
    {
      jtl::immutable_string path;
      std::unordered_map<std::string, loaded_object_symbol> symbols;
    };

    /* Runtime symbol metadata captured from JITLink once ORC assigns final addresses. This is the
     * bridge between a raw PC in a stack trace and the original object-file symbol metadata above. */
    struct materialized_symbol
    {
      uptr resource_key{};
      uptr runtime_address{};
      usize runtime_size{};
      jtl::immutable_string object_path;
      uptr object_address{};
      usize object_size{};
      jtl::immutable_string symbol;
    };

    void register_loaded_object(uptr resource_key, loaded_object object) const;
    void transfer_loaded_object(uptr dst_resource_key, uptr src_resource_key) const;
    void remove_loaded_object(uptr resource_key) const;
    void record_materialized_symbol(uptr resource_key,
                                    std::string const &symbol,
                                    uptr runtime_address,
                                    usize runtime_size) const;
    void remove_materialized_symbols(uptr resource_key) const;
    void transfer_materialized_symbols(uptr dst_resource_key, uptr src_resource_key) const;

    /* Resource key -> original object file metadata captured at `load_object` time. */
    mutable folly::Synchronized<std::unordered_map<uptr, loaded_object>> loaded_objects;
    /* Runtime symbol start address -> materialized symbol metadata captured from JITLink. */
    mutable folly::Synchronized<std::map<uptr, materialized_symbol>> materialized_symbols;
  };
}
