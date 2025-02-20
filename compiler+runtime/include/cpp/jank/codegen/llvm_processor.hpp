#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::runtime::obj
{
  using keyword_ptr = native_box<struct keyword>;
}

namespace jank::codegen
{
  using namespace jank::runtime;

  enum class compilation_target : uint8_t
  {
    module,
    function,
    eval
  };

  struct reusable_context
  {
    reusable_context(native_persistent_string const &module_name);

    native_persistent_string module_name;
    native_persistent_string ctor_name;

    std::unique_ptr<llvm::LLVMContext> llvm_ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    llvm::Value *nil{};
    llvm::BasicBlock *global_ctor_block{};

    /* TODO: Is this needed, given lifted constants? */
    native_unordered_map<object_ptr, llvm::Value *, std::hash<object_ptr>, very_equal_to>
      literal_globals;
    native_unordered_map<obj::symbol_ptr, llvm::Value *> var_globals;
    native_unordered_map<native_persistent_string, llvm::Value *> c_string_globals;

    /* Optimization details. */
    std::unique_ptr<llvm::FunctionPassManager> fpm;
    std::unique_ptr<llvm::LoopAnalysisManager> lam;
    std::unique_ptr<llvm::FunctionAnalysisManager> fam;
    std::unique_ptr<llvm::CGSCCAnalysisManager> cgam;
    std::unique_ptr<llvm::ModuleAnalysisManager> mam;
    std::unique_ptr<llvm::PassInstrumentationCallbacks> pic;
    std::unique_ptr<llvm::StandardInstrumentations> si;
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
    /* For this ctor, we're inheriting the context from another function, which means
     * we're building a nested function. */
    llvm_processor(analyze::expr::function<analyze::expression> const &expr,
                   std::unique_ptr<reusable_context> ctx);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    string_result<void> gen();
    llvm::Value *gen(analyze::expression_ptr const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::def<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::var_deref<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &) const;
    llvm::Value *gen(analyze::expr::var_ref<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &) const;
    llvm::Value *gen(analyze::expr::call<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::primitive_literal<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::list<analyze::expression> const &,
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
    llvm::Value *gen(analyze::expr::recursion_reference<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::named_recursion<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::let<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::letfn<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::do_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::if_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::throw_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::try_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);
    llvm::Value *gen(analyze::expr::case_<analyze::expression> const &,
                     analyze::expr::function_arity<analyze::expression> const &);

    llvm::Value *gen_var(obj::symbol_ptr qualified_name) const;
    llvm::Value *gen_c_string(native_persistent_string const &s) const;

    native_persistent_string to_string() const;

    void create_function();
    void create_function(analyze::expr::function_arity<analyze::expression> const &arity);
    void create_global_ctor();
    llvm::GlobalVariable *create_global_var(native_persistent_string const &name) const;

    llvm::Value *gen_global(obj::nil_ptr) const;
    llvm::Value *gen_global(obj::boolean_ptr b) const;
    llvm::Value *gen_global(obj::integer_ptr i) const;
    llvm::Value *gen_global(obj::real_ptr r) const;
    llvm::Value *gen_global(obj::ratio_ptr r) const;
    llvm::Value *gen_global(obj::persistent_string_ptr s) const;
    llvm::Value *gen_global(obj::symbol_ptr s);
    llvm::Value *gen_global(obj::keyword_ptr k) const;
    llvm::Value *gen_global(obj::character_ptr c) const;
    llvm::Value *gen_global_from_read_string(object_ptr o);
    llvm::Value *
    gen_function_instance(analyze::expr::function<analyze::expression> const &expr,
                          analyze::expr::function_arity<analyze::expression> const &fn_arity);

    llvm::StructType *get_or_insert_struct_type(std::string const &name,
                                                std::vector<llvm::Type *> const &fields) const;

    compilation_target target{};
    /* This is stored just to keep the expression alive. */
    analyze::expression_ptr root_expr{};
    analyze::expr::function<analyze::expression> const &root_fn;
    llvm::Function *fn{};
    std::unique_ptr<reusable_context> ctx;
    native_unordered_map<obj::symbol_ptr, llvm::Value *> locals;
  };
}
