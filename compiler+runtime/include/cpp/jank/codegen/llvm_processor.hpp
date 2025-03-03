#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>

#include <jank/analyze/processor.hpp>

namespace jank::runtime::obj
{
  using keyword_ptr = native_box<struct keyword>;
}

namespace jank::analyze
{
  using expression_ptr = runtime::native_box<struct expression>;

  namespace expr
  {
    using def_ptr = runtime::native_box<struct def>;
    using var_deref_ptr = runtime::native_box<struct var_deref>;
    using var_ref_ptr = runtime::native_box<struct var_ref>;
    using call_ptr = runtime::native_box<struct call>;
    using primitive_literal_ptr = runtime::native_box<struct primitive_literal>;
    using list_ptr = runtime::native_box<struct list>;
    using vector_ptr = runtime::native_box<struct vector>;
    using map_ptr = runtime::native_box<struct map>;
    using set_ptr = runtime::native_box<struct set>;
    using local_reference_ptr = runtime::native_box<struct local_reference>;
    using function_ptr = runtime::native_box<struct function>;
    using recur_ptr = runtime::native_box<struct recur>;
    using recursion_reference_ptr = runtime::native_box<struct recursion_reference>;
    using named_recursion_ptr = runtime::native_box<struct named_recursion>;
    using let_ptr = runtime::native_box<struct let>;
    using letfn_ptr = runtime::native_box<struct letfn>;
    using do_ptr = runtime::native_box<struct do_>;
    using if_ptr = runtime::native_box<struct if_>;
    using throw_ptr = runtime::native_box<struct throw_>;
    using try_ptr = runtime::native_box<struct try_>;
    using case_ptr = runtime::native_box<struct case_>;
  }
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
    native_unordered_map<runtime::object_ptr,
                         llvm::Value *,
                         std::hash<runtime::object_ptr>,
                         very_equal_to>
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
    llvm_processor(analyze::expr::function_ptr const expr,
                   native_persistent_string const &module,
                   compilation_target target);
    /* For this ctor, we're inheriting the context from another function, which means
     * we're building a nested function. */
    llvm_processor(analyze::expr::function_ptr expr, std::unique_ptr<reusable_context> ctx);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    string_result<void> gen();
    llvm::Value *gen(analyze::expression_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::def_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::var_deref_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::var_ref_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::call_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::primitive_literal_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::list_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::vector_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::map_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::set_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::local_reference_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::function_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::recur_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::recursion_reference_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::named_recursion_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::let_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::letfn_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::do_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::if_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::throw_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::try_ptr, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::case_ptr, analyze::expr::function_arity const &);

    llvm::Value *gen_var(obj::symbol_ptr qualified_name) const;
    llvm::Value *gen_c_string(native_persistent_string const &s) const;

    native_persistent_string to_string() const;

    void create_function();
    void create_function(analyze::expr::function_arity const &arity);
    void create_global_ctor() const;
    llvm::GlobalVariable *create_global_var(native_persistent_string const &name) const;

    llvm::Value *gen_global(runtime::obj::nil_ptr) const;
    llvm::Value *gen_global(runtime::obj::boolean_ptr b) const;
    llvm::Value *gen_global(runtime::obj::integer_ptr i) const;
    llvm::Value *gen_global(runtime::obj::real_ptr r) const;
    llvm::Value *gen_global(runtime::obj::ratio_ptr r) const;
    llvm::Value *gen_global(runtime::obj::persistent_string_ptr s) const;
    llvm::Value *gen_global(runtime::obj::symbol_ptr s);
    llvm::Value *gen_global(runtime::obj::keyword_ptr k) const;
    llvm::Value *gen_global(runtime::obj::character_ptr c) const;
    llvm::Value *gen_global_from_read_string(runtime::object_ptr o);
    llvm::Value *gen_function_instance(analyze::expr::function_ptr expr,
                                       analyze::expr::function_arity const &fn_arity);

    llvm::StructType *get_or_insert_struct_type(std::string const &name,
                                                std::vector<llvm::Type *> const &fields) const;

    compilation_target target{};
    analyze::expr::function_ptr root_fn{};
    llvm::Function *fn{};
    std::unique_ptr<reusable_context> ctx;
    native_unordered_map<obj::symbol_ptr, llvm::Value *> locals;
  };
}
