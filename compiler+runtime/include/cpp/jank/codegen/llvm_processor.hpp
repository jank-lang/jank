#pragma once

#include <jtl/ptr.hpp>

#include <jank/analyze/processor.hpp>

namespace llvm::orc
{
  class ThreadSafeModule;
}

namespace jank::codegen
{
  using namespace jank::runtime;

  enum class compilation_target : u8
  {
    module,
    function,
    eval
  };

  constexpr char const *compilation_target_str(compilation_target const t)
  {
    switch(t)
    {
      case compilation_target::module:
        return "module";
      case compilation_target::function:
        return "function";
      case compilation_target::eval:
        return "eval";
      default:
        return "unknown";
    }
  }

  enum class var_root_kind : u8;

  struct reusable_context;

  struct llvm_processor
  {
    struct impl;

    llvm_processor() = delete;
    llvm_processor(analyze::expr::function_ref const expr,
                   jtl::immutable_string const &module,
                   compilation_target target);
    /* For this ctor, we're inheriting the context from another function, which means
     * we're building a nested function. */
    llvm_processor(analyze::expr::function_ref expr, jtl::ref<reusable_context> ctx);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    jtl::string_result<void> gen() const;
    void optimize() const;
    void print() const;

    llvm::orc::ThreadSafeModule &get_module() const;
    jtl::immutable_string const &get_module_name() const;
    jtl::immutable_string get_root_fn_name() const;

    jtl::ptr<impl> _impl{};
  };
}
