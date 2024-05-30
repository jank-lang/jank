#pragma once

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>

namespace jank::runtime
{
  struct context;
}

namespace jank::codegen::llvm
{
  /* Codegen processors render a single function expression to a C++ functor. REPL expressions
   * are wrapped in a nullary functor. These functors nest arbitrarily, if an expression has more
   * fn values of its own, each one rendered with its own codegen processor. */
  struct processor
  {
    processor() = delete;
    processor(runtime::context &rt_ctx,
              analyze::expression_ptr const &expr,
              native_persistent_string_view const &module,
              compilation_target target);
    processor(runtime::context &rt_ctx,
              analyze::expr::function<analyze::expression> const &expr,
              native_persistent_string_view const &module,
              compilation_target target);
    processor(processor const &) = delete;
    processor(processor &&) noexcept = default;

    ::llvm::Value *gen(analyze::expression_ptr const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::def<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::var_deref<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::var_ref<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::call<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::primitive_literal<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::vector<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::map<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::set<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::local_reference const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::function<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::recur<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::let<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::do_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::if_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::throw_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::try_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);
    ::llvm::Value *gen(analyze::expr::native_raw<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &);

    void gen_body();

    runtime::context &rt_ctx;
    /* This is stored just to keep the expression alive. */
    analyze::expression_ptr root_expr{};
    analyze::expr::function<analyze::expression> const &root_fn;
    native_persistent_string module;
    compilation_target target{};
    runtime::obj::symbol struct_name;

    std::unique_ptr<::llvm::LLVMContext> jit_context;
    std::unique_ptr<::llvm::Module> jit_module;
    std::unique_ptr<::llvm::IRBuilder<>> jit_builder;
  };
}
