#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::codegen
{
  using namespace jank::runtime;

  struct llvm_processor
  {
    llvm_processor() = delete;
    llvm_processor(analyze::expression_ptr const &expr,
                   native_persistent_string const &module,
                   compilation_target target);
    llvm_processor(analyze::expr::function<analyze::expression> const &expr,
                   native_persistent_string const &module,
                   compilation_target target);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    void gen();
    llvm::Value *gen(analyze::expression_ptr const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::def<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::var_deref<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::var_ref<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::call<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::primitive_literal<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::vector<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::map<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::set<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::local_reference const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::function<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::recur<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::let<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::do_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::if_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::throw_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::try_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::native_raw<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);

    llvm::Value *gen_var(obj::symbol_ptr qualified_name);

    native_persistent_string to_string();

    void create_function();
    void install_global_ctors();
    llvm::Value *nil_global();
    llvm::Value *string_global(obj::persistent_string_ptr const s);

    /* This is stored just to keep the expression alive. */
    analyze::expression_ptr root_expr{};
    analyze::expr::function<analyze::expression> const &root_fn;
    native_persistent_string module_name;
    compilation_target target{};
    native_persistent_string struct_name;
    native_persistent_string expression_fn_name;

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    llvm::Function *fn{};
    llvm::Value *nil{};
    native_unordered_map<object_ptr, llvm::Value *> literal_globals;
    native_unordered_map<obj::symbol_ptr, llvm::Value *> var_globals;
    native_vector<llvm::Function *> global_ctors;
    llvm::BasicBlock *global_ctor_block{};
  };
}
