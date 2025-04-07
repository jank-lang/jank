#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>

#include <jtl/ptr.hpp>

#include <jank/analyze/processor.hpp>

namespace jank::runtime::obj
{
  using nil_ref = jtl::object_ref<struct nil>;
  using keyword_ref = jtl::object_ref<struct keyword>;
  using boolean_ref = jtl::object_ref<struct boolean>;
  using integer_ref = jtl::object_ref<struct integer>;
  using real_ref = jtl::object_ref<struct real>;
  using ratio_ref = jtl::object_ref<struct ratio>;
  using persistent_string_ref = jtl::object_ref<struct persistent_string>;
  using character_ref = jtl::object_ref<struct character>;
}

namespace jank::analyze
{
  using expression_ref = jtl::ref<struct expression>;

  namespace expr
  {
    using def_ref = jtl::ref<struct def>;
    using var_deref_ref = jtl::ref<struct var_deref>;
    using var_ref_ref = jtl::ref<struct var_ref>;
    using call_ref = jtl::ref<struct call>;
    using primitive_literal_ref = jtl::ref<struct primitive_literal>;
    using list_ref = jtl::ref<struct list>;
    using vector_ref = jtl::ref<struct vector>;
    using map_ref = jtl::ref<struct map>;
    using set_ref = jtl::ref<struct set>;
    using local_reference_ref = jtl::ref<struct local_reference>;
    using function_ref = jtl::ref<struct function>;
    using recur_ref = jtl::ref<struct recur>;
    using recursion_reference_ref = jtl::ref<struct recursion_reference>;
    using named_recursion_ref = jtl::ref<struct named_recursion>;
    using let_ref = jtl::ref<struct let>;
    using do_ref = jtl::ref<struct do_>;
    using if_ref = jtl::ref<struct if_>;
    using throw_ref = jtl::ref<struct throw_>;
    using try_ref = jtl::ref<struct try_>;
    using case_ref = jtl::ref<struct case_>;
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
    reusable_context(jtl::immutable_string const &module_name);

    jtl::immutable_string module_name;
    jtl::immutable_string ctor_name;

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
    native_unordered_map<obj::symbol_ref, llvm::Value *> var_globals;
    native_unordered_map<jtl::immutable_string, llvm::Value *> c_string_globals;

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
    llvm_processor(analyze::expr::function_ref const expr,
                   jtl::immutable_string const &module,
                   compilation_target target);
    /* For this ctor, we're inheriting the context from another function, which means
     * we're building a nested function. */
    llvm_processor(analyze::expr::function_ref expr, std::unique_ptr<reusable_context> ctx);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    jtl::string_result<void> gen();
    llvm::Value *gen(analyze::expression_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::def_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::var_deref_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::var_ref_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::call_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::primitive_literal_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::list_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::vector_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::map_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::set_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::local_reference_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::function_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::recur_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::recursion_reference_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::named_recursion_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::let_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::do_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::if_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::throw_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::try_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::case_ref, analyze::expr::function_arity const &);

    llvm::Value *gen_var(obj::symbol_ref qualified_name) const;
    llvm::Value *gen_c_string(jtl::immutable_string const &s) const;

    jtl::immutable_string to_string() const;

    void create_function();
    void create_function(analyze::expr::function_arity const &arity);
    void create_global_ctor() const;
    llvm::GlobalVariable *create_global_var(jtl::immutable_string const &name) const;

    llvm::Value *gen_global(runtime::obj::nil_ref) const;
    llvm::Value *gen_global(runtime::obj::boolean_ref b) const;
    llvm::Value *gen_global(runtime::obj::integer_ref i) const;
    llvm::Value *gen_global(runtime::obj::real_ref r) const;
    llvm::Value *gen_global(runtime::obj::ratio_ref r) const;
    llvm::Value *gen_global(runtime::obj::persistent_string_ref s) const;
    llvm::Value *gen_global(runtime::obj::symbol_ref s);
    llvm::Value *gen_global(runtime::obj::keyword_ref k) const;
    llvm::Value *gen_global(runtime::obj::character_ref c) const;
    llvm::Value *gen_global_from_read_string(runtime::object_ptr o);
    llvm::Value *gen_function_instance(analyze::expr::function_ref expr,
                                       analyze::expr::function_arity const &fn_arity);

    llvm::StructType *get_or_insert_struct_type(std::string const &name,
                                                std::vector<llvm::Type *> const &fields) const;

    compilation_target target{};
    analyze::expr::function_ref root_fn;
    jtl::ptr<llvm::Function> fn{};
    std::unique_ptr<reusable_context> ctx;
    native_unordered_map<obj::symbol_ref, llvm::Value *> locals;
  };
}
