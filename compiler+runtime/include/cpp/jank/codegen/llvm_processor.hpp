#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::codegen
{
  using namespace jank::runtime;

  struct nested_tag
  {
  };

  struct llvm_processor
  {
    llvm_processor() = delete;
    llvm_processor(analyze::expression_ptr const &expr,
                   native_persistent_string const &module,
                   compilation_target target);
    llvm_processor(analyze::expr::function<analyze::expression> const &expr,
                   native_persistent_string const &module,
                   compilation_target target);
    llvm_processor(nested_tag,
                   analyze::expr::function<analyze::expression> const &expr,
                   llvm_processor &&);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    void release(llvm_processor &into) &&;

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
    llvm::Value *gen_c_string(native_persistent_string const &s);

    native_persistent_string to_string();

    void create_function();
    void create_function(analyze::expr::function_arity<analyze::expression> const &arity);
    void create_global_ctor();
    llvm::GlobalVariable *create_global_var(native_persistent_string const &name);

    llvm::Value *gen_global(obj::nil_ptr);
    llvm::Value *gen_global(obj::boolean_ptr b);
    llvm::Value *gen_global(obj::integer_ptr i);
    llvm::Value *gen_global(obj::real_ptr r);
    llvm::Value *gen_global(obj::persistent_string_ptr s);
    llvm::Value *gen_global(obj::symbol_ptr s);
    llvm::Value *gen_global(obj::keyword_ptr k);
    llvm::Value *gen_global(obj::character_ptr c);
    llvm::Value *gen_global_from_read_string(object_ptr o);

    llvm::StructType *
    get_or_insert_struct_type(std::string const &name, std::vector<llvm::Type *> const &fields);

    /* This is stored just to keep the expression alive. */
    analyze::expression_ptr root_expr{};
    analyze::expr::function<analyze::expression> const &root_fn;
    native_persistent_string module_name;
    compilation_target target{};
    native_persistent_string struct_name;
    native_persistent_string ctor_name;

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    llvm::Function *fn{};
    llvm::Value *nil{};
    native_unordered_map<object_ptr, llvm::Value *> literal_globals;
    native_unordered_map<obj::symbol_ptr, llvm::Value *> var_globals;
    native_unordered_map<native_persistent_string, llvm::Value *> c_string_globals;
    llvm::BasicBlock *global_ctor_block{};
    native_unordered_map<obj::symbol_ptr, llvm::Value *> locals;
  };
}
