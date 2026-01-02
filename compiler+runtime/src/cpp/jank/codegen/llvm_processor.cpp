#include <list>

#include <Interpreter/Compatibility.h>
#include <clang/Interpreter/CppInterOp.h>

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/rtti.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/profile/time.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/cli.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>

/* TODO: Remove exceptions. */
namespace jank::codegen
{
  using namespace jank::analyze;

  struct deferred_init
  {
    analyze::expr::function_ref expr;
    obj::symbol_ref name;
    analyze::local_binding_ptr binding;
    llvm::Value *field_ptr{};
  };

  struct reusable_context
  {
    reusable_context(jtl::immutable_string const &module_name,
                     std::unique_ptr<llvm::LLVMContext> llvm_ctx);

    jtl::immutable_string module_name;
    jtl::immutable_string ctor_name;

    llvm::orc::ThreadSafeModule module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    llvm::BasicBlock *global_ctor_block{};

    /* TODO: Is this needed, given lifted constants? */
    native_unordered_map<runtime::object_ref,
                         llvm::Value *,
                         std::hash<runtime::object_ref>,
                         very_equal_to>
      literal_globals;
    native_unordered_map<obj::symbol_ref, llvm::Value *> var_globals;
    native_unordered_map<obj::symbol_ref, llvm::Value *> var_root_globals;
    native_unordered_map<jtl::immutable_string, llvm::Value *> c_string_globals;

    /* Optimization details. */
    std::unique_ptr<llvm::LoopAnalysisManager> lam;
    std::unique_ptr<llvm::FunctionAnalysisManager> fam;
    std::unique_ptr<llvm::CGSCCAnalysisManager> cgam;
    std::unique_ptr<llvm::ModuleAnalysisManager> mam;
    std::unique_ptr<llvm::PassInstrumentationCallbacks> pic;
    std::unique_ptr<llvm::StandardInstrumentations> si;
    llvm::ModulePassManager mpm;
  };

  struct llvm_processor::impl
  {
    impl(analyze::expr::function_ref const expr,
         jtl::immutable_string const &module,
         compilation_target target);
    /* For this ctor, we're inheriting the context from another function, which means
     * we're building a nested function. */
    impl(analyze::expr::function_ref expr, jtl::ref<reusable_context> ctx);

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
    llvm::Value *gen(analyze::expr::letfn_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::do_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::if_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::throw_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::try_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::case_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_raw_ref, analyze::expr::function_arity const &);
    static llvm::Value *gen(analyze::expr::cpp_type_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_value_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_cast_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_call_ref, analyze::expr::function_arity const &);
    llvm::Value *
    gen(analyze::expr::cpp_constructor_call_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_constructor_call_ref,
                     analyze::expr::function_arity const &,
                     llvm::Value *alloc);
    llvm::Value *gen(analyze::expr::cpp_member_call_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_member_access_ref, analyze::expr::function_arity const &);
    llvm::Value *
    gen(analyze::expr::cpp_builtin_operator_call_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_box_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_unbox_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_new_ref, analyze::expr::function_arity const &);
    llvm::Value *gen(analyze::expr::cpp_delete_ref, analyze::expr::function_arity const &);

    llvm::Value *gen_var(obj::symbol_ref const qualified_name) const;
    llvm::Value *gen_var_root(obj::symbol_ref const qualified_name, var_root_kind kind) const;
    llvm::Value *gen_c_string(jtl::immutable_string const &s) const;

    void create_function();
    void create_function(analyze::expr::function_arity const &arity);
    void create_global_ctor() const;
    llvm::GlobalVariable *create_global_var(jtl::immutable_string const &name) const;

    llvm::Value *gen_global(runtime::obj::nil_ref const) const;
    llvm::Value *gen_global(runtime::obj::boolean_ref const b) const;
    llvm::Value *gen_global(runtime::obj::integer_ref const i) const;
    llvm::Value *gen_global(runtime::obj::big_integer_ref const i) const;
    llvm::Value *gen_global(runtime::obj::big_decimal_ref const i) const;
    llvm::Value *gen_global(runtime::obj::real_ref const r) const;
    llvm::Value *gen_global(runtime::obj::ratio_ref const r) const;
    llvm::Value *gen_global(runtime::obj::persistent_string_ref const s) const;
    llvm::Value *gen_global(runtime::obj::symbol_ref const s) const;
    llvm::Value *gen_global(runtime::obj::keyword_ref const k) const;
    llvm::Value *gen_global(runtime::obj::character_ref const c) const;
    llvm::Value *gen_global(runtime::obj::re_pattern_ref const re) const;
    llvm::Value *gen_global(runtime::obj::uuid_ref const u) const;
    llvm::Value *gen_global(runtime::obj::inst_ref const i) const;
    llvm::Value *gen_global_from_read_string(runtime::object_ref const o) const;
    llvm::Value *gen_function_instance(analyze::expr::function_ref expr,
                                       analyze::expr::function_arity const &fn_arity);
    llvm::Value *gen_aot_call(Cpp::AotCall const &call,
                              jtl::ptr<void> const fn,
                              jtl::ptr<void> const expr_type,
                              jtl::immutable_string const &name,
                              native_vector<analyze::expression_ref> const &arg_exprs,
                              analyze::expression_position const position,
                              analyze::expression_kind const kind,
                              analyze::expr::function_arity const &arity);
    llvm::Value *gen_aot_call(Cpp::AotCall const &call,
                              llvm::Value *ret_alloc,
                              jtl::ptr<void> const fn,
                              jtl::ptr<void> const expr_type,
                              jtl::immutable_string const &name,
                              native_vector<analyze::expression_ref> const &arg_exprs,
                              analyze::expression_position const position,
                              analyze::expression_kind const kind,
                              analyze::expr::function_arity const &arity);

    llvm::StructType *get_or_insert_struct_type(std::string const &name,
                                                std::vector<llvm::Type *> const &fields) const;

    util::scope_exit gen_stack_save();
    void gen_stack_restore();
    jtl::ptr<llvm::Value> gen_ret(jtl::ptr<llvm::Value> const value);
    jtl::ptr<llvm::Value> gen_ret();

    compilation_target target{};
    analyze::expr::function_ref root_fn;
    jtl::ptr<llvm::Function> llvm_fn{};
    jtl::ref<reusable_context> ctx;
    native_unordered_map<obj::symbol_ref, jtl::ptr<llvm::Value>> locals;
    native_list<deferred_init> deferred_inits{};
    jtl::ref<llvm::LLVMContext> llvm_ctx;
    jtl::ref<llvm::Module> llvm_module;
    jtl::ptr<llvm::BasicBlock> current_loop;

    /* Landingpad and catch basic block used for try...catch...finally */
    struct lpad_and_catch_bb
    {
      llvm::BasicBlock *lpad_bb{};
      llvm::BasicBlock *catch_bb{};
    };

    native_vector<lpad_and_catch_bb> lpad_and_catch_body_stack{};
    /* These are the registered RTTI modules compiled as part of this fn.
     * We don't use this within the current fn, but it's passed upward to
     * the fn gen which is above us, all the way up to the module level. */
    native_unordered_map<jtl::immutable_string, Cpp::AotCall> global_rtti;
    native_vector<jtl::ptr<llvm::Value>> stack_saves;
  };

  struct llvm_type_info
  {
    jtl::ref<llvm::Type> type;
    usize size{};
    usize alignment{};
  };

  static jtl::immutable_string unique_munged_string()
  {
    return runtime::munge(__rt_ctx->unique_namespaced_string());
  }

  static jtl::immutable_string unique_munged_string(jtl::immutable_string const &prefix)
  {
    return runtime::munge(__rt_ctx->unique_namespaced_string(prefix));
  }

  static llvm::Type *llvm_builtin_type(reusable_context const &ctx,
                                       jtl::ref<llvm::LLVMContext> const llvm_ctx,
                                       jtl::ptr<void> const type)
  {
    auto &interp{ __rt_ctx->jit_prc.interpreter };
    auto const ci{ interp->getCI() };
    auto const qtype{ clang::QualType::getFromOpaquePtr(type) };
    if(qtype->isIntegerType())
    {
      return ctx.builder->getIntNTy(ci->getASTContext().getTypeSize(qtype));
    }
    else if(qtype->isRealFloatingType())
    {
      switch(ci->getASTContext().getTypeSize(qtype))
      {
        case 16:
          return ctx.builder->getHalfTy();
        case 32:
          return ctx.builder->getFloatTy();
        case 64:
          return ctx.builder->getDoubleTy();
        case 128:
          return llvm::Type::getFP128Ty(*llvm_ctx);
        default:
          break;
      }
    }
    jank_assert_fmt_throw(false,
                          "Unable to find LLVM IR primitive for type '{}'.",
                          Cpp::GetTypeAsString(type));
  }

  static llvm_type_info llvm_type(reusable_context const &ctx,
                                  jtl::ref<llvm::LLVMContext> const llvm_ctx,
                                  jtl::ptr<void> const type)
  {
    jank_debug_assert(type);
    usize size{ 1 };
    //util::println("alloc_type {}, size {}", Cpp::GetTypeAsString(type), size);
    jank_debug_assert(size > 0);
    auto const alignment{ Cpp::GetAlignmentOfType(type) };
    jank_debug_assert(alignment > 0);
    llvm::Type *ir_type{ ctx.builder->getInt8Ty() };

    if(Cpp::IsPointerType(type) || Cpp::IsReferenceType(type) || Cpp::IsArrayType(type))
    {
      ir_type = ctx.builder->getPtrTy();
    }
    else if(Cpp::IsBuiltin(type))
    {
      ir_type = llvm_builtin_type(ctx, llvm_ctx, type);
    }
    else if(Cpp::IsEnumType(type))
    {
      ir_type = llvm_builtin_type(ctx, llvm_ctx, Cpp::GetIntegerTypeFromEnumType(type));
    }
    else if(auto scope = Cpp::GetScopeFromType(type); scope && Cpp::IsClass(scope))
    {
      /* TODO: We need the IR name, not the C++ name. */
      ir_type = llvm::StructType::getTypeByName(*llvm_ctx, Cpp::GetQualifiedName(scope));
      if(!ir_type)
      {
        ir_type = ctx.builder->getInt8Ty();
        size = Cpp::GetSizeOfType(type);
      }
    }

    /* Our special wrapper refs/ptrs are indistinguishable from raw pointers at the byte level. */
    if(cpp_util::is_any_object(type))
    {
      ir_type = ctx.builder->getPtrTy();
      size = 1;
    }

    jank_debug_assert_fmt_throw(ir_type,
                                "Unable to find LLVM IR primitive to use for allocating type '{}'.",
                                Cpp::GetTypeAsString(type));

    return { ir_type, size, alignment };
  }

  static llvm::Value *alloc_type(reusable_context const &ctx,
                                 jtl::ref<llvm::LLVMContext> const llvm_ctx,
                                 jtl::ptr<void> const type,
                                 jtl::immutable_string const &name = "")
  {
    auto const type_info{ llvm_type(ctx, llvm_ctx, type) };
    auto const ir_size{ llvm::ConstantInt::get(ctx.builder->getInt64Ty(), type_info.size) };
    auto const alloc{ ctx.builder->CreateAlloca(type_info.type.data, ir_size, name.c_str()) };
    alloc->setAlignment(llvm::Align{ type_info.alignment });
    return alloc;
  }

  static void link_module(reusable_context &ctx, jtl::ref<llvm::Module> const raw_module)
  {
    auto const tsc{ ctx.module.getContext() };
    //std::unique_ptr<llvm::Module> module{ raw_module };
    //llvm::orc::ThreadSafeModule tsm{ jtl::move(module), tsc };
    auto cloned_cpp_module{ llvm::orc::cloneExternalModuleToContext(*raw_module, tsc) };
    cloned_cpp_module.consumingModuleDo([&](std::unique_ptr<llvm::Module> module) {
      llvm::Linker::linkModules(*ctx.module.getModuleUnlocked(), jtl::move(module));
    });
  }

  static u8 pointer_count(jtl::ptr<void> type)
  {
    u8 ret{};
    if(Cpp::IsReferenceType(type))
    {
      ++ret;
      type = Cpp::GetNonReferenceType(type);
    }
    while(Cpp::IsPointerType(type))
    {
      ++ret;
      type = Cpp::GetPointeeType(type);
    }
    if(cpp_util::is_any_object(type))
    {
      ++ret;
    }

    return ret;
  }

  /* Generates the IR to call into jank's conversion trait to convert to/from
   * an object, based on the `policy`. We need to know the input's type, the
   * expected output type, as well as the type to use to instantiate the
   * conversion trait. It's common for these to overlap, but they may be different. */
  static llvm::Value *convert_object(reusable_context &ctx,
                                     jtl::ref<llvm::LLVMContext> const llvm_ctx,
                                     jtl::ref<llvm::Module> const llvm_module,
                                     conversion_policy const policy,
                                     jtl::ptr<void> const input_type,
                                     jtl::ptr<void> const output_type,
                                     jtl::ptr<void> conversion_type,
                                     llvm::Value * const arg)
  {
    /* TODO: If output is void, just return nil. */
    conversion_type = Cpp::GetNonReferenceType(conversion_type);

    if(cpp_util::is_typed_object(Cpp::GetNonReferenceType(input_type))
       && cpp_util::is_untyped_object(Cpp::GetNonReferenceType(output_type)))
    {
      auto const base_offset{ cpp_util::offset_to_typed_object_base(
        Cpp::GetNonReferenceType(input_type)) };
      if(0 < base_offset)
      {
        auto const arg_base{ ctx.builder->CreateInBoundsGEP(
          ctx.builder->getInt8Ty(),
          arg,
          { ctx.builder->getInt64(base_offset) },
          util::format("{}.base", arg->getName().str()).c_str()) };
        return arg_base;
      }
      return arg;
    }

    /* References to arrays are just treated as pointers, since that's the decayed type.
     * That requires a bit of a dance here, though. */
    auto const is_arg_ref{ Cpp::IsReferenceType(input_type)
                           && !(Cpp::IsArrayType(Cpp::GetNonReferenceType(input_type))) };
    auto const is_arg_ptr{ Cpp::IsPointerType(input_type)
                           || (Cpp::IsArrayType(Cpp::GetNonReferenceType(input_type))) };
    //util::println(
    //  "convert_object input_type = {}, output_type = {}, conversion_type = {}, policy = {}",
    //  Cpp::GetTypeAsString(input_type),
    //  Cpp::GetTypeAsString(output_type),
    //  Cpp::GetTypeAsString(conversion_type),
    //  conversion_policy_str(policy));
    static auto const convert_template{ Cpp::GetScopeFromCompleteName("jank::runtime::convert") };
    Cpp::TemplateArgInfo const template_arg{ Cpp::GetTypeWithoutCv(conversion_type) };
    auto const instantiation{ Cpp::InstantiateTemplate(convert_template, &template_arg, 1) };
    jank_debug_assert(instantiation);
    auto const conversion_fns{ Cpp::GetFunctionsUsingName(
      instantiation,
      policy == conversion_policy::into_object ? "into_object" : "from_object") };
    std::vector<Cpp::TemplateArgInfo> input_args;
    if(!Cpp::IsVoid(input_type))
    {
      input_args.emplace_back(Cpp::GetTypeWithoutCv(input_type));
    }
    auto const match{ Cpp::BestOverloadFunctionMatch(conversion_fns, {}, input_args) };
    if(!match)
    {
      throw std::runtime_error{ util::format(
        "Unable to find conversion function match for policy '{}' with conversion type '{}' and "
        "input type '{}'.",
        policy == conversion_policy::into_object ? "into_object" : "from_object",
        Cpp::GetTypeAsString(conversion_type),
        Cpp::GetTypeAsString(input_type)) };
    }
    auto const match_name{ Cpp::GetCompleteName(match) };
    auto const param_type{ Cpp::GetFunctionArgType(match, 0) };
    auto const is_param_indirect{ param_type
                                  && (Cpp::IsReferenceType(param_type)
                                      || Cpp::IsPointerType(param_type)
                                      /*|| Cpp::IsArrayType(param_type)*/) };

    auto const fn_callable{ Cpp::MakeAotCallable(match, unique_munged_string()) };
    link_module(ctx, reinterpret_cast<llvm::Module *>(fn_callable.getModule()));

    llvm::Value *arg_alloc{ arg };
    if(cpp_util::is_any_object(input_type) && !llvm::isa<llvm::AllocaInst>(arg_alloc))
    {
      arg_alloc = ctx.builder->CreateAlloca(ctx.builder->getPtrTy(),
                                            llvm::ConstantInt::get(ctx.builder->getInt32Ty(), 1));
      ctx.builder->CreateStore(arg, arg_alloc);
    }
    else if((is_arg_ref && llvm::isa<llvm::AllocaInst>(arg))
            || (is_arg_ptr && !is_param_indirect && llvm::isa<llvm::AllocaInst>(arg)))
    {
      arg_alloc = ctx.builder->CreateLoad(ctx.builder->getPtrTy(), arg_alloc);
    }

    auto const fn(llvm_module->getFunction(fn_callable.getName()));
    auto const args_array_type{ llvm::ArrayType::get(ctx.builder->getPtrTy(), 1) };
    auto const args_array{ ctx.builder->CreateAlloca(args_array_type,
                                                     nullptr,
                                                     util::format("{}.args", match_name).c_str()) };
    auto const arg_array_0{ ctx.builder->CreateInBoundsGEP(
      args_array_type,
      args_array,
      { ctx.builder->getInt32(0), ctx.builder->getInt32(0) },
      util::format("{}.args[{}]", match_name, 0).c_str()) };
    ctx.builder->CreateStore(arg_alloc, arg_array_0);

    auto const ret_alloc{
      alloc_type(ctx, llvm_ctx, output_type, util::format("{}.alloc", match_name).c_str())
    };

    llvm::SmallVector<llvm::Value *, 4> const args{
      llvm::ConstantPointerNull::get(ctx.builder->getPtrTy()),
      llvm::ConstantInt::getSigned(ctx.builder->getInt32Ty(), 1),
      args_array,
      ret_alloc
    };
    ctx.builder->CreateCall(fn, args);
    if(policy == conversion_policy::from_object)
    {
      return ret_alloc;
    }

    auto const load_ret{ ctx.builder->CreateLoad(ctx.builder->getPtrTy(),
                                                 ret_alloc,
                                                 util::format("{}.res", match_name).c_str()) };

    /* If it's a typed object, erase it. */
    auto const ret_type{ Cpp::GetFunctionReturnType(match) };
    if(!cpp_util::is_typed_object(ret_type))
    {
      return load_ret;
    }

    /* No need to call a function to erase a typed object. Just find the
     * offset to its base member and shift our pointer accordingly. */
    auto const base_offset{ cpp_util::offset_to_typed_object_base(ret_type) };
    //util::println("convert_object ret_type = {}, needs base adjustment by {} bytes",
    //              Cpp::GetTypeAsString(ret_type),
    //              base_offset);
    if(0 < base_offset)
    {
      auto const ret_base{ ctx.builder->CreateInBoundsGEP(
        ctx.builder->getInt8Ty(),
        load_ret,
        { ctx.builder->getInt64(base_offset) },
        util::format("{}.base", load_ret->getName().str()).c_str()) };
      return ret_base;
    }

    return load_ret;
  }

  static jtl::ref<llvm::LLVMContext> extract_context(llvm::orc::ThreadSafeModule const &module)
  {
    jtl::ptr<llvm::LLVMContext> raw_ctx;
    module.getContext().withContextDo([&](auto const ctx) { raw_ctx = ctx; });
    return raw_ctx.data;
  }

  /* Whenever we have an object in an `alloca`, we need to load it before using. This fn only
   * makes sense to use with jank objects, as opposed to native values. */
  static llvm::Value *
  load_if_needed(jtl::ref<reusable_context> const ctx, llvm::Value *arg, jtl::ptr<void> const type)
  {
    if(!arg)
    {
      return arg;
    }

    if(llvm::isa<llvm::AllocaInst>(arg) && cpp_util::is_any_object(type))
    {
      arg = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), arg);
    }
    return arg;
  }

  static llvm::Value *load_if_needed(jtl::ref<reusable_context> const ctx, llvm::Value * const arg)
  {
    return load_if_needed(ctx, arg, cpp_util::untyped_object_ref_type());
  }

  reusable_context::reusable_context(jtl::immutable_string const &module_name,
                                     std::unique_ptr<llvm::LLVMContext> llvm_ctx)
    : module_name{ module_name }
    , ctor_name{ unique_munged_string("jank_global_init") }
    //, llvm_ctx{ std::make_unique<llvm::LLVMContext>() }
    //, llvm_ctx{ reinterpret_cast<std::unique_ptr<llvm::orc::ThreadSafeContext> *>(
    //              reinterpret_cast<void *>(
    //                &static_cast<clang::Interpreter &>(*__rt_ctx->jit_prc.interpreter)))
    //              ->getContext() }
    , lam{ std::make_unique<llvm::LoopAnalysisManager>() }
    , fam{ std::make_unique<llvm::FunctionAnalysisManager>() }
    , cgam{ std::make_unique<llvm::CGSCCAnalysisManager>() }
    , mam{ std::make_unique<llvm::ModuleAnalysisManager>() }
    , pic{ std::make_unique<llvm::PassInstrumentationCallbacks>() }
  {
    auto m{ std::make_unique<llvm::Module>(unique_munged_string(module_name).c_str(), *llvm_ctx) };
    module = llvm::orc::ThreadSafeModule{ std::move(m), std::move(llvm_ctx) };

    auto const raw_ctx{ extract_context(module) };
    builder = std::make_unique<llvm::IRBuilder<>>(*raw_ctx);
    global_ctor_block = llvm::BasicBlock::Create(*raw_ctx, "entry");
    si = std::make_unique<llvm::StandardInstrumentations>(*raw_ctx,
                                                          /*DebugLogging*/ false);

    /* The LLVM front-end tips documentation suggests setting the target triple and
     * data layout to improve back-end codegen performance. */
    auto const raw_module{ module.getModuleUnlocked() };
    raw_module->setTargetTriple(llvm::Triple{ util::default_target_triple().c_str() });
    raw_module->setDataLayout(__rt_ctx->jit_prc.interpreter->getExecutionEngine()->getDataLayout());

    /* TODO: Add more passes and measure the order of the passes. */

    si->registerCallbacks(*pic, mam.get());

    llvm::PassBuilder pb;
    pb.registerModuleAnalyses(*mam);
    pb.registerCGSCCAnalyses(*cgam);
    pb.registerFunctionAnalyses(*fam);
    pb.registerLoopAnalyses(*lam);
    pb.crossRegisterProxies(*lam, *fam, *cgam, *mam);
    /* TODO: Configure this level based on the CLI optimization flag.
     * Benchmark to find the best default. */
    mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
  }

  /* There are three places where a var-root could be generated,
   * depending on different circumstances.
   *
   * 1. global_init: Initialized and derefed in the global ctor.
   *    This represents the "eval" case. When evaling a jank form at the top level
   *    with direct calls enabled, the var-root is initialized in the global ctor.
   *
   * 2. binded_def: Initialized right after the call to jank_var_bind_root,
   *    using the value as the value of the global var-root.
   *    This applies to ahead-of-time compiled modules.
   *
   * 3. load_init: Initialized and derefed in the "jank_load" IR function.
   *    This, like 2, applies to ahead-of-time compiled modules.
   */
  enum class var_root_kind : u8
  {
    global_init,
    binded_def,
    load_init
  };

  llvm_processor::llvm_processor(expr::function_ref const expr,
                                 jtl::immutable_string const &module_name,
                                 compilation_target const target)
    : _impl{ make_ref<impl>(expr, module_name, target) }
  {
  }

  llvm_processor::llvm_processor(expr::function_ref const expr,
                                 jtl::ref<reusable_context> const ctx)
    : _impl{ make_ref<impl>(expr, ctx) }
  {
  }

  llvm_processor::impl::impl(expr::function_ref const expr,
                             jtl::immutable_string const &module_name,
                             compilation_target const target)
    : target{ target }
    , root_fn{ expr }
    , ctx{ make_ref<reusable_context>(module_name, std::make_unique<llvm::LLVMContext>()) }
    , llvm_ctx{ extract_context(ctx->module) }
    , llvm_module{ ctx->module.getModuleUnlocked() }
  {
  }

  llvm_processor::impl::impl(expr::function_ref const expr, jtl::ref<reusable_context> ctx)
    : target{ compilation_target::function }
    , root_fn{ expr }
    , ctx{ std::move(ctx) }
    , llvm_ctx{ extract_context(this->ctx->module) }
    , llvm_module{ this->ctx->module.getModuleUnlocked() }
  {
  }

  void llvm_processor::impl::create_function()
  {
    auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), false));
    auto const name(munge(root_fn->unique_name));
    llvm_fn = llvm::Function::Create(fn_type,
                                     llvm::Function::ExternalLinkage,
                                     name.c_str(),
                                     *llvm_module);

    auto const entry(llvm::BasicBlock::Create(*llvm_ctx, "entry", llvm_fn));
    ctx->builder->SetInsertPoint(entry);
  }

  void llvm_processor::impl::create_function(expr::function_arity const &arity)
  {
    auto const captures(root_fn->captures());
    auto const is_closure(!captures.empty());

    /* Closures get one extra parameter, the first one, which is a pointer to the closure's
     * context. The context is a struct containing all captured values.
     *
     * We add one unconditionally for the `this` object. */
    std::vector<llvm::Type *> const arg_types{ arity.params.size() + 1, ctx->builder->getPtrTy() };
    auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
    std::string const name{ munge(root_fn->unique_name) };
    auto const fn_name{ target == compilation_target::module
                          ? jtl::immutable_string{ name }
                          : util::format("{}_{}", name, arity.params.size()) };
    auto fn_value(llvm_module->getOrInsertFunction(fn_name.c_str(), fn_type));
    llvm_fn = llvm::cast<llvm::Function>(fn_value.getCallee());
    llvm_fn->setLinkage(llvm::Function::ExternalLinkage);

    auto const entry(llvm::BasicBlock::Create(*llvm_ctx, "entry", llvm_fn));
    ctx->builder->SetInsertPoint(entry);

    /* JIT-loaded object files don't support global ctors, so we need to call ours manually.
     * Fortunately, we have our load function, which we can hook into. So, if we're compiling
     * a module, and we've just created the load function for that module, the first thing
     * we want to do is call our global ctor. */
    if(target == compilation_target::module
       && root_fn->unique_name == module::module_to_load_function(ctx->module_name))
    {
      auto const global_ctor_fn(ctx->global_ctor_block->getParent());
      ctx->builder->CreateCall(global_ctor_fn, {});

      /* This dance is performed to keep symbol names unique across all the modules.
       * Considering LLVM JIT symbols to be global, we need to define them with
       * unique names to avoid conflicts during JIT recompilation/reloading.
       *
       * The approach, right now, is for each namespace, we will keep a counter
       * and will increase it every time we define a new symbol. When we JIT reload
       * the same namespace again, we will define new symbols.
       *
       * This IR codegen for calling `jank_ns_set_symbol_counter`, is to set the counter
       * on an initial load.
       */
      auto const current_ns{ __rt_ctx->current_ns() };
      auto const func_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getInt64Ty() },
                                false));
      auto const fn(llvm_module->getOrInsertFunction("jank_ns_set_symbol_counter", func_type));

      ctx->builder->CreateCall(
        fn,
        { gen_c_string(current_ns->name->get_name()),
          llvm::ConstantInt::get(ctx->builder->getInt64Ty(), current_ns->symbol_counter.load()) });
    }

    auto this_arg(llvm_fn->getArg(0));
    this_arg->setName("this");
    /* We need a way to represent the current object, but we don't want to conflict with
     * any existing symbols in the scope, so we introduce a qualified symbol. This will be
     * impossible to have as a local, so it's safe. */
    locals[make_box<obj::symbol>("virtual/this")] = this_arg;
    locals[make_box<obj::symbol>(root_fn->name)] = this_arg;

    for(usize i{}; i < arity.params.size(); ++i)
    {
      auto &param(arity.params[i]);
      auto arg(llvm_fn->getArg(i + 1));
      arg->setName(param->get_name().c_str());
      locals[param] = arg;
    }

    if(is_closure)
    {
      static constexpr auto offset_of_base{ offsetof(runtime::obj::jit_closure, base) };
      static constexpr auto offset_of_context{ offsetof(runtime::obj::jit_closure, context) };
      jank_debug_assert(offset_of_base < offset_of_context);
      static constexpr auto offset_of_context_from_base{ offset_of_context - offset_of_base };

      auto const context_ptr{ ctx->builder->CreateInBoundsGEP(
        ctx->builder->getInt8Ty(),
        this_arg,
        { ctx->builder->getInt64(offset_of_context_from_base) }) };
      auto const context{
        ctx->builder->CreateLoad(ctx->builder->getPtrTy(), context_ptr, "this.context")
      };
      auto const capture_list(root_fn->captures());
      std::vector<llvm::Type *> const capture_types{ capture_list.size(),
                                                     ctx->builder->getPtrTy() };
      auto const closure_ctx_type(
        get_or_insert_struct_type(util::format("{}_context", munge(root_fn->unique_name)),
                                  capture_types));
      usize index{};
      for(auto const &capture : capture_list)
      {
        auto const field_ptr(ctx->builder->CreateStructGEP(closure_ctx_type, context, index++));
        locals[capture.first] = ctx->builder->CreateLoad(ctx->builder->getPtrTy(),
                                                         field_ptr,
                                                         capture.first->name.c_str());
      }
    }
  }

  jtl::string_result<void> llvm_processor::gen() const
  {
    return _impl->gen();
  }

  jtl::string_result<void> llvm_processor::impl::gen()
  {
    profile::timer const timer{ util::format("ir gen {}", root_fn->name) };
    if(target != compilation_target::function)
    {
      create_global_ctor();
    }

    for(auto const &arity : root_fn->arities)
    {
      /* TODO: Add profiling to the fn body? Need to exit on every return. */
      create_function(arity);
      bool block_terminated{};
      for(auto const form : arity.body->values)
      {
        gen(form, arity);
        if(ctx->builder->GetInsertBlock()->getTerminator())
        {
          block_terminated = true;
          break;
        }
      }

      /* If the inner loop was terminated, we skip the final check. */
      if(block_terminated)
      {
        continue;
      }

      gen_ret(gen_global(jank_nil()));
    }

    if(target != compilation_target::function)
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      if(profile::is_enabled())
      {
        auto const fn_type(
          llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false));
        auto const fn(llvm_module->getOrInsertFunction("jank_profile_exit", fn_type));
        ctx->builder->CreateCall(
          fn,
          { gen_c_string(util::format("global ctor for {}", root_fn->name)) });
      }

      gen_ret();
    }

    /* For modules, we need to make sure to define RTTI symbols manually. Since
     * nested fns will introduce these, we have a way to float those up
     * from those fns all the way here to the module level. Here, we just need
     * to link those modules into our own to get the necessary type info
     * globals defined. */
    if(target == compilation_target::module)
    {
      for(auto const &rtti : global_rtti)
      {
        link_module(*ctx, reinterpret_cast<llvm::Module *>(rtti.second.getModule()));
      }
    }

    return ok();
  }

  llvm::Value *
  llvm_processor::impl::gen(expression_ref const ex, expr::function_arity const &fn_arity)
  {
    llvm::Value *ret{};
    visit_expr([&](auto const typed_ex) { ret = gen(typed_ex, fn_arity); }, ex);
    return ret;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::def_ref const expr, expr::function_arity const &arity)
  {
    auto const ref(gen_var(expr->name));

    if(expr->value.is_some())
    {
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const fn(llvm_module->getOrInsertFunction("jank_var_bind_root", fn_type));
      auto const var_root(gen(expr->value.unwrap(), arity));
      llvm::SmallVector<llvm::Value *, 2> const args{ ref, var_root };
      ctx->builder->CreateCall(fn, args);

      /* Here if compiling into a jank module, a var-root can be initialized directly with the parameter
       * being used by the call to "jank_var_bind_root". There is no deref needed. */
      if(util::cli::opts.direct_call && target == compilation_target::module)
      {
        auto const global_var_root(gen_var_root(expr->name, var_root_kind::binded_def));
        ctx->builder->CreateStore(var_root, global_var_root);
      }
    }

    if(expr->name->meta.is_some())
    {
      auto const set_meta_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_meta_fn(llvm_module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

      auto const meta_val(
        gen_global_from_read_string(strip_source_from_meta(expr->name->meta.unwrap())));
      ctx->builder->CreateCall(set_meta_fn, { ref, meta_val });
    }

    auto const set_dynamic_fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(),
                              { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                              false));

    auto const set_dynamic_fn(
      llvm_module->getOrInsertFunction("jank_var_set_dynamic", set_dynamic_fn_type));

    auto const dynamic{ truthy(get(expr->name->meta.unwrap_or(jank_nil()),
                                   __rt_ctx->intern_keyword("dynamic").expect_ok())) };

    auto const dynamic_global{ gen_global(make_box(dynamic)) };

    ctx->builder->CreateCall(set_dynamic_fn, { ref, dynamic_global });

    if(expr->position == expression_position::tail)
    {
      return gen_ret(ref);
    }

    return ref;
  }

  llvm::Value *
  /* NOLINTNEXTLINE(readability-make-member-function-const): Can't be const, due to overload resolution ambiguities. */
  llvm_processor::impl::gen(expr::var_deref_ref const expr, expr::function_arity const &)
  {
    llvm::Value *call{};
    auto const var_qualified_name(make_box<obj::symbol>(expr->var->n, expr->var->name));

    /* For direct-calls, when derefing a var, we need to handle two different types of var-derefs.
     * We only direct-call vars that are not dynamic.
     * When generating the IR for a var-root, if the function is a jank_load function,
     * the var_root is derefed directly in the "jank_load" function.
     * If it is any other function, the var_root is initialized and derefed in the global ctor. */
    if(util::cli::opts.direct_call && !(expr->var->dynamic))
    {
      if(root_fn->name.starts_with("jank_load"))
      {
        call = gen_var_root(var_qualified_name, var_root_kind::load_init);
      }
      else
      {
        call = gen_var_root(var_qualified_name, var_root_kind::global_init);
      }
    }
    else
    {
      auto const ref(gen_var(var_qualified_name));
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const fn(llvm_module->getOrInsertFunction("jank_deref", fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ ref };
      call = ctx->builder->CreateCall(fn, args);
    }
    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  /* NOLINTNEXTLINE(readability-make-member-function-const): Can't be const, due to overload resolution ambiguities. */
  llvm::Value *llvm_processor::impl::gen(expr::var_ref_ref const expr, expr::function_arity const &)
  {
    auto const var(gen_var(expr->qualified_name));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(var);
    }

    return var;
  }

  static jtl::immutable_string arity_to_call_fn(usize const arity)
  {
    /* Anything max_params + 1 or higher gets packed into a list, so we
     * just end up calling max_params + 1 at most. */
    switch(arity)
    {
      case 0 ... runtime::max_params:
        return util::format("jank_call{}", arity);
      default:
        return util::format("jank_call{}", runtime::max_params + 1);
    }
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::call_ref const expr, expr::function_arity const &arity)
  {
    auto const callee(gen(expr->source_expr, arity));

    llvm::SmallVector<llvm::Value *> arg_handles;
    llvm::SmallVector<llvm::Type *> arg_types;
    /* We add one for the fn object. */
    arg_handles.reserve(expr->arg_exprs.size() + 1);
    arg_types.reserve(expr->arg_exprs.size() + 1);

    llvm::Value *call{};
    if(cpp_util::is_any_object(cpp_util::expression_type(expr->source_expr)))
    {
      arg_handles.emplace_back(callee);
      arg_types.emplace_back(ctx->builder->getPtrTy());

      for(auto const &arg_expr : expr->arg_exprs)
      {
        auto arg_handle{ gen(arg_expr, arity) };
        if(llvm::isa<llvm::AllocaInst>(arg_handle))
        {
          arg_handle = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), arg_handle);
        }

        arg_handles.emplace_back(arg_handle);
        arg_types.emplace_back(ctx->builder->getPtrTy());
      }

      auto const call_fn_name(arity_to_call_fn(expr->arg_exprs.size()));
      auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
      auto const fn(llvm_module->getOrInsertFunction(call_fn_name.c_str(), fn_type));

      if(lpad_and_catch_body_stack.empty())
      {
        call = ctx->builder->CreateCall(fn, arg_handles);
      }
      else
      {
        auto const normal_dest{ llvm::BasicBlock::Create(
          *llvm_ctx,
          util::format("invoke.{}.normal", call_fn_name).data(),
          llvm_fn) };
        call = ctx->builder->CreateInvoke(fn,
                                          normal_dest,
                                          lpad_and_catch_body_stack.back().lpad_bb,
                                          arg_handles);
        ctx->builder->SetInsertPoint(normal_dest);
      }
    }
    /* TODO: This can be deleted, I'm pretty sure. */
    else
    {
      throw std::runtime_error{ "oops, thought this was dead code" };
    }

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::primitive_literal_ref const expr, expr::function_arity const &)
  {
    auto const ret(runtime::visit_object(
      [&](auto const typed_o) -> llvm::Value * {
        using T = decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, runtime::obj::nil> || std::same_as<T, runtime::obj::boolean>
                     || std::same_as<T, runtime::obj::integer>
                     || std::same_as<T, runtime::obj::real> || std::same_as<T, runtime::obj::symbol>
                     || std::same_as<T, runtime::obj::character>
                     || std::same_as<T, runtime::obj::keyword>
                     || std::same_as<T, runtime::obj::persistent_string>
                     || std::same_as<T, runtime::obj::ratio>
                     || std::same_as<T, runtime::obj::big_integer>
                     || std::same_as<T, runtime::obj::big_decimal>
                     || std::same_as<T, runtime::obj::uuid> || std::same_as<T, runtime::obj::inst>
                     || std::same_as<T, runtime::obj::re_pattern>)
        {
          return gen_global(typed_o);
        }
        else if constexpr(std::same_as<T, runtime::obj::persistent_vector>
                          || std::same_as<T, runtime::obj::persistent_list>
                          || std::same_as<T, runtime::obj::persistent_hash_set>
                          || std::same_as<T, runtime::obj::persistent_array_map>
                          || std::same_as<T, runtime::obj::persistent_hash_map>
                          /* Cons, etc. */
                          || runtime::behavior::seqable<T>)
        {
          return gen_global_from_read_string(typed_o);
        }
        else
        {
          throw std::runtime_error{ util::format("Unimplemented constant codegen: {}\n",
                                                 typed_o->to_string()) };
        }
      },
      expr->data));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(ret);
    }

    return ret;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::list_ref const expr, expr::function_arity const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(llvm_module->getOrInsertFunction("jank_list_create", fn_type));

    auto const size(expr->data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + size);
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &data_expr : expr->data_exprs)
    {
      args.emplace_back(load_if_needed(ctx, gen(data_expr, arity)));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::vector_ref const expr, expr::function_arity const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(llvm_module->getOrInsertFunction("jank_vector_create", fn_type));

    auto const size(expr->data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + size);
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &data_expr : expr->data_exprs)
    {
      args.emplace_back(load_if_needed(ctx, gen(data_expr, arity)));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::map_ref const expr, expr::function_arity const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(llvm_module->getOrInsertFunction("jank_map_create", fn_type));

    auto const size(expr->data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + (size * 2));
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &pair : expr->data_exprs)
    {
      args.emplace_back(load_if_needed(ctx, gen(pair.first, arity)));
      args.emplace_back(load_if_needed(ctx, gen(pair.second, arity)));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::set_ref const expr, expr::function_arity const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(llvm_module->getOrInsertFunction("jank_set_create", fn_type));

    auto const size(expr->data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + size);
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &data_expr : expr->data_exprs)
    {
      args.emplace_back(load_if_needed(ctx, gen(data_expr, arity)));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::impl::gen(expr::local_reference_ref const expr,
                                         [[maybe_unused]] expr::function_arity const &arity)
  {
    auto ret(locals[expr->binding->name]);
    jank_debug_assert_fmt(ret,
                          "Unable to find binding for local '{}' in fn '{}'",
                          expr->binding->name->to_code_string(),
                          arity.fn_ctx->name);

    if(expr->position == expression_position::tail)
    {
      return gen_ret(load_if_needed(ctx, ret));
    }

    return ret;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::function_ref const expr, expr::function_arity const &fn_arity)
  {
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };

      llvm_processor const nested{ expr, ctx };
      auto const res{ nested.gen() };
      if(res.is_err())
      {
        /* TODO: Return error. */
        res.expect_ok();
      }

      global_rtti.insert(nested._impl->global_rtti.begin(), nested._impl->global_rtti.end());
    }

    auto const fn_obj(gen_function_instance(expr, fn_arity));

    if(expr->position == expression_position::tail)
    {
      return gen_ret(fn_obj);
    }

    return fn_obj;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::recur_ref const expr, expr::function_arity const &arity)
  {
    /* Using `recur` in a loop will just mean calculating the new values for the
     * loop binding's `alloca` and then storing the values. We store all values at the
     * end, since the old values must be used for all calculations of the new values. */
    if(expr->loop_target.is_some())
    {
      native_vector<std::pair<llvm::Value *, llvm::Value *>> deferred_stores;
      for(usize i{}; i < expr->arg_exprs.size(); ++i)
      {
        auto const &arg_expr{ expr->arg_exprs[i] };
        auto arg_handle{ gen(arg_expr, arity) };
        auto const expr_type{ cpp_util::expression_type(arg_expr) };
        auto const is_arg_ref{ Cpp::IsReferenceType(expr_type)
                               && !(Cpp::IsPointerType(Cpp::GetNonReferenceType(expr_type))
                                    || Cpp::IsArrayType(Cpp::GetNonReferenceType(expr_type))) };
        auto const is_arg_ptr{ Cpp::IsPointerType(expr_type) || Cpp::IsArrayType(expr_type)
                               || cpp_util::is_any_object(expr_type) };
        auto const is_arg_indirect{ is_arg_ref || is_arg_ptr };

        if(is_arg_indirect && llvm::isa<llvm::AllocaInst>(arg_handle))
        {
          arg_handle = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), arg_handle);
        }

        auto const arg_alloc{ locals[expr->loop_target.unwrap()->pairs[i].first] };
        jank_debug_assert(arg_alloc);
        deferred_stores.emplace_back(arg_handle, arg_alloc);
      }

      for(auto const &store : deferred_stores)
      {
        ctx->builder->CreateStore(store.first, store.second);
      }

      gen_stack_restore();
      return ctx->builder->CreateBr(current_loop.data);
    }
    else
    {
      /* The codegen for the special recur form is very similar to the named recursion
     * codegen, but it's simpler. The key difference is that named recursion requires
     * arg packing, whereas the special recur form does not. This means, for variadic
     * functions, the special recur form will be expected to supply a sequence for the
     * variadic argument. */
      auto const &fn_expr(*root_fn->arities[0].fn_ctx->fn);

      llvm::SmallVector<llvm::Value *> arg_handles;
      llvm::SmallVector<llvm::Type *> arg_types;
      /* We add one for `this`. */
      arg_handles.reserve(expr->arg_exprs.size() + 1);
      arg_types.reserve(expr->arg_exprs.size() + 1);

      arg_handles.emplace_back(ctx->builder->GetInsertBlock()->getParent()->getArg(0));
      arg_types.emplace_back(ctx->builder->getPtrTy());

      for(auto const &arg_expr : expr->arg_exprs)
      {
        auto arg_handle{ gen(arg_expr, arity) };
        auto const expr_type{ cpp_util::expression_type(arg_expr) };
        auto const is_arg_ref{ Cpp::IsReferenceType(expr_type)
                               && !(Cpp::IsPointerType(Cpp::GetNonReferenceType(expr_type))
                                    || Cpp::IsArrayType(Cpp::GetNonReferenceType(expr_type))) };
        auto const is_arg_ptr{ Cpp::IsPointerType(expr_type) || Cpp::IsArrayType(expr_type)
                               || cpp_util::is_any_object(expr_type) };
        auto const is_arg_indirect{ is_arg_ref || is_arg_ptr };

        if(is_arg_indirect && llvm::isa<llvm::AllocaInst>(arg_handle))
        {
          arg_handle = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), arg_handle);
        }

        arg_handles.emplace_back(arg_handle);
        arg_types.emplace_back(ctx->builder->getPtrTy());
      }

      auto const call_fn_name(
        util::format("{}_{}", munge(fn_expr.unique_name), expr->arg_exprs.size()));
      auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
      auto const fn(llvm_module->getOrInsertFunction(call_fn_name.c_str(), fn_type));
      auto const call(ctx->builder->CreateCall(fn, arg_handles));
      call->setTailCall();

      if(expr->position == expression_position::tail)
      {
        return gen_ret(call);
      }

      return call;
    }
  }

  llvm::Value *llvm_processor::impl::gen(expr::recursion_reference_ref const expr,
                                         expr::function_arity const &arity)
  {
    auto const crosses_fn(expr->fn_ctx->fn != arity.fn_ctx->fn);

    llvm::Value *fn_obj{ locals[make_box<obj::symbol>("virtual/this")] };
    if(crosses_fn)
    {
      fn_obj = locals[make_box<obj::symbol>(expr->fn_ctx->fn->name)];
    }

    if(expr->position == expression_position::tail)
    {
      return gen_ret(fn_obj);
    }

    return fn_obj;
  }

  /* Named recursion is a special kind of call. We can't always go through a var, since there
   * may not be one. We can't just use the fn's name, since we could be recursing into a
   * different arity.
   *
   * For named recursion calls, we don't use dynamic_call. We just call the generated C fn
   * directly. This doesn't impede interactivity, since the whole thing will be redefined
   * if a new fn is created. */
  llvm::Value *
  llvm_processor::impl::gen(expr::named_recursion_ref const expr, expr::function_arity const &arity)
  {
    auto const &fn_expr(*expr->recursion_ref.fn_ctx->fn);

    /* We may have a named recursion in a closure which crosses another function to
     * recurse. For example,
     *
     * ```clojure
     * (let [a 1]
     *   (fn foo []
     *     (fn bar []
     *       (boop a)
     *       (foo))))
     * ```
     *
     * Here, the `(foo)` call is a named recursion, but we're not in the `foo` fn.
     * We need to "cross" `bar` to get back into `foo`. This is an important
     * distinction, since the closure context for `foo` and `bar` may be different, such
     * as if `bar` closes over more data than `foo` does.
     *
     * In this case of a named recursion which crosses a fn, we can't use the current fn's
     * closure context. */

    auto const crosses_fn(expr->recursion_ref.fn_ctx->fn != arity.fn_ctx->fn);

    llvm::SmallVector<llvm::Value *> arg_handles;
    llvm::SmallVector<llvm::Type *> arg_types;
    /* We add one for `this`. */
    arg_handles.reserve(expr->arg_exprs.size() + 1);
    arg_types.reserve(expr->arg_exprs.size() + 1);

    arg_handles.emplace_back(locals[make_box<obj::symbol>("virtual/this")]);
    arg_types.emplace_back(ctx->builder->getPtrTy());

    if(crosses_fn)
    {
      arg_handles[0] = locals[make_box<obj::symbol>(expr->recursion_ref.fn_ctx->fn->name)];
    }

    for(auto const &arg_expr : expr->arg_exprs)
    {
      arg_handles.emplace_back(load_if_needed(ctx, gen(arg_expr, arity)));
      arg_types.emplace_back(ctx->builder->getPtrTy());
    }

    llvm::Value *call{};
    if(expr->recursion_ref.fn_ctx->is_variadic)
    {
      auto const call_fn_name(arity_to_call_fn(expr->arg_exprs.size()));
      auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
      auto const fn(llvm_module->getOrInsertFunction(call_fn_name.c_str(), fn_type));
      call = ctx->builder->CreateCall(fn, arg_handles);
    }
    else
    {
      auto const call_fn_name(
        util::format("{}_{}", munge(fn_expr.unique_name), expr->arg_exprs.size()));
      auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
      auto const fn(llvm_module->getOrInsertFunction(call_fn_name.c_str(), fn_type));
      call = ctx->builder->CreateCall(fn, arg_handles);
    }

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::let_ref const expr, expr::function_arity const &arity)
  {
    auto old_locals(locals);
    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      if(local.is_none())
      {
        throw std::runtime_error{ util::format("ICE: unable to find local: {}",
                                               pair.first->to_string()) };
      }

      auto const value{ gen(pair.second, arity) };
      locals[pair.first] = value;
      if(value->getName().empty())
      {
        value->setName(util::format("{}_init", pair.first->to_string()).c_str());
      }
    }

    /* Loops are implemented by creating an `alloca` for each binding, which will be a mutable
     * container for the value as it changes each iteration. We then create a new basic block
     * for the loop body and a final post-loop block for where to jump when we're done.
     *
     * Whenever we hit a `recur`, the new values are calculated and then stored into the
     * corresponding `alloca` before jumping back to the start of the loop block. */
    if(expr->is_loop)
    {
      for(auto const &pair : expr->pairs)
      {
        auto const alloc{ ctx->builder->CreateAlloca(
          ctx->builder->getPtrTy(),
          llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
        alloc->setName(pair.first->to_string().c_str());
        ctx->builder->CreateStore(load_if_needed(ctx, locals[pair.first]), alloc);
        locals[pair.first] = alloc;
      }

      auto const current_fn(ctx->builder->GetInsertBlock()->getParent());
      auto const loop_block(llvm::BasicBlock::Create(*llvm_ctx, "loop", current_fn));
      auto const old_loop{ current_loop };
      current_loop = loop_block;
      util::scope_exit const finally{ [&]() { current_loop = old_loop; } };

      ctx->builder->CreateBr(loop_block);
      ctx->builder->SetInsertPoint(loop_block);

      auto const stack_save{ gen_stack_save() };

      auto const ret(gen(expr->body, arity));
      locals = std::move(old_locals);

      /* XXX: No return creation, since we rely on the body to do that. */
      if(expr->position != expression_position::tail)
      {
        auto const postloop_block(llvm::BasicBlock::Create(*llvm_ctx, "postloop", current_fn));
        if(!ctx->builder->GetInsertBlock()->getTerminator())
        {
          gen_stack_restore();
          ctx->builder->CreateBr(postloop_block);
        }
        ctx->builder->SetInsertPoint(postloop_block);
      }

      return ret;
    }
    else
    {
      auto const ret(gen(expr->body, arity));
      locals = std::move(old_locals);

      /* XXX: No return creation, since we rely on the body to do that. */

      return ret;
    }
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::letfn_ref const expr, expr::function_arity const &arity)
  {
    /* We generate bindings left-to-right, so for mutually recursive letfn bindings
     * we must defer some initialization via `deferred_inits`.
     *
     * In the following example, `b` is easy to generate since `a` is already initialized at line 6.
     * However, `b` is not available when initializing `a` at line 2, so it is moved to line 8.
     *
     *   (jank.compiler/native-source '(letfn [(a [] b) (b [] a)]))
     *   =>
     *   1 | %1 = call ptr @GC_malloc(i64 8)
     *   2 | ...                                                     // %b not in scope. store moved to line 8
     *   3 | %a = call ptr @jank_closure_create(..., ptr %1)
     *   4 | ...
     *   5 | %4 = call ptr @GC_malloc(i64 8)
     *   6 | store ptr %a, ptr %4, align 8                           // %a is in scope, ok to store
     *   7 | %b = call ptr @jank_closure_create(..., ptr nonnull %4)
     *   8 | store ptr %b, ptr %1, align 8                           // deferred initialization of %a since %b is in scope
     * */
    auto old_deferred_inits(deferred_inits);
    deferred_inits = {};

    auto old_locals(locals);
    for(auto const &pair : expr->pairs)
    {
      auto const local(expr->frame->find_local_or_capture(pair.first));
      if(local.is_none())
      {
        throw std::runtime_error{ util::format("ICE: unable to find local: {}",
                                               pair.first->to_string()) };
      }

      locals[pair.first] = gen(pair.second, arity);
      locals[pair.first]->setName(pair.first->to_string().c_str());
    }

    for(auto const &deferred_init : deferred_inits)
    {
      expr::local_reference const local_ref{ expression_position::value,
                                             deferred_init.expr->frame,
                                             deferred_init.expr->needs_box,
                                             deferred_init.name,
                                             deferred_init.binding };
      auto const e(gen(expr::local_reference_ref{ &local_ref }, arity));
      ctx->builder->CreateStore(e, deferred_init.field_ptr);
    }

    auto const ret(gen(expr->body, arity));
    locals = std::move(old_locals);
    deferred_inits = std::move(old_deferred_inits);

    /* XXX: No return creation, since we rely on the body to do that. */

    return ret;
  }

  llvm::Value *llvm_processor::impl::gen(expr::do_ref const expr, expr::function_arity const &arity)
  {
    /* NOLINTNEXTLINE(misc-const-correctness): Cant' be const. */
    llvm::Value *last{};
    for(auto const &form : expr->values)
    {
      last = gen(form, arity);
    }

    /* Codegen for this already generated a return. */
    return last;
  }

  llvm::Value *llvm_processor::impl::gen(expr::if_ref const expr, expr::function_arity const &arity)
  {
    /* If we're in the return position, our then/else branches will generate return instructions
     * for us. Since LLVM basic blocks can only have one terminating instruction, we need
     * to take care to not generate our own, too. */
    auto const is_return(expr->position == expression_position::tail);
    auto const condition(load_if_needed(ctx, gen(expr->condition, arity)));
    auto const truthy_fn_type(
      llvm::FunctionType::get(ctx->builder->getInt8Ty(), { ctx->builder->getPtrTy() }, false));
    auto const fn(llvm_module->getOrInsertFunction("jank_truthy", truthy_fn_type));
    llvm::SmallVector<llvm::Value *, 1> const args{ condition };
    auto const call(ctx->builder->CreateCall(fn, args));
    auto const cmp(ctx->builder->CreateICmpEQ(call, ctx->builder->getInt8(1), "iftmp"));

    auto const current_fn(ctx->builder->GetInsertBlock()->getParent());
    auto then_block(llvm::BasicBlock::Create(*llvm_ctx, "then", current_fn));
    auto else_block(llvm::BasicBlock::Create(*llvm_ctx, "else"));
    auto const merge_block(llvm::BasicBlock::Create(*llvm_ctx, "postif"));
    auto const if_type{ cpp_util::expression_type(expr) };

    ctx->builder->CreateCondBr(cmp, then_block, else_block);

    ctx->builder->SetInsertPoint(then_block);
    auto const then(load_if_needed(ctx, gen(expr->then, arity), if_type));

    if(!is_return && !ctx->builder->GetInsertBlock()->getTerminator())
    {
      ctx->builder->CreateBr(merge_block);
    }

    /* Codegen for `then` can change the current block, so track that. */
    then_block = ctx->builder->GetInsertBlock();
    current_fn->insert(current_fn->end(), else_block);

    ctx->builder->SetInsertPoint(else_block);
    llvm::Value *else_{};

    if(expr->else_.is_some())
    {
      else_ = load_if_needed(ctx, gen(expr->else_.unwrap(), arity), if_type);
    }
    else
    {
      else_ = gen_global(jank_nil());
      if(expr->position == expression_position::tail)
      {
        else_ = gen_ret(else_);
      }
    }

    if(!is_return && !ctx->builder->GetInsertBlock()->getTerminator())
    {
      ctx->builder->CreateBr(merge_block);
    }

    /* Codegen for `else` can change the current block, so track that. */
    else_block = ctx->builder->GetInsertBlock();

    if(!is_return)
    {
      current_fn->insert(current_fn->end(), merge_block);
      ctx->builder->SetInsertPoint(merge_block);

      /* If we're leaving a branch from then/else, we don't need a phi, since we only have
       * one value to select. This can happen in a loop, for example, where the `then` will
       * always just `recur` (which leads to a branch) and only the `else` actually produces a
       * value. */
      if(llvm::isa<llvm::BranchInst>(then))
      {
        return else_;
      }
      else if(llvm::isa<llvm::BranchInst>(else_))
      {
        return then;
      }
      else
      {
        auto const phi(
          ctx->builder->CreatePHI(is_return ? ctx->builder->getVoidTy() : ctx->builder->getPtrTy(),
                                  2,
                                  "iftmp"));
        phi->addIncoming(then, then_block);
        phi->addIncoming(else_, else_block);

        return phi;
      }
    }
    return nullptr;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::throw_ref const expr, expr::function_arity const &arity)
  {
    auto const value(gen(expr->value, arity));
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false));
    auto fn(llvm_module->getOrInsertFunction("jank_throw", fn_type));
    llvm::cast<llvm::Function>(fn.getCallee())->setDoesNotReturn();

    if(!lpad_and_catch_body_stack.empty())
    {
      auto const unreachable_dest{
        llvm::BasicBlock::Create(*llvm_ctx, "unreachable.throw", llvm_fn)
      };
      ctx->builder->CreateInvoke(fn,
                                 unreachable_dest,
                                 lpad_and_catch_body_stack.back().lpad_bb,
                                 { value });
      ctx->builder->SetInsertPoint(unreachable_dest);
    }
    else
    {
      ctx->builder->CreateCall(fn, { value });
    }

    /* Since this code path never completes, it doesn't matter what we return.
     * Using `jank_nil` to satisfy some IR requirements. */
    auto const ret{ gen_global(jank_nil()) };
    if(expr->position == expression_position::tail)
    {
      return gen_ret(ret);
    }
    return ret;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::try_ref const expr, expr::function_arity const &arity)
  {
    if(expr->catch_body.is_none() && expr->finally_body.is_none())
    {
      return gen(expr->body, arity);
    }

    auto const current_fn(ctx->builder->GetInsertBlock()->getParent());
    auto &entry_bb{ current_fn->getEntryBlock() };
    llvm::IRBuilder<> entry_builder(&entry_bb, entry_bb.getFirstInsertionPt());
    auto const ptr_ty{ ctx->builder->getPtrTy() };

    if(!current_fn->hasPersonalityFn())
    {
      /* To signal to the unwinder that this fn can handle exceptions, we must have a
       * personality function registered. The personality function tells the unwinder if this fn can
       * (or cannot) handle a specific exception. Once the unwinder finds a match, it transfers
       * control flow to the exception handling code. The personality function then populates the
       * exception information and transfers control flow to the landing pad block. */
      auto personality_fn_type{ llvm::FunctionType::get(ctx->builder->getInt32Ty(),
                                                        /*isVarArg=*/true) };
      auto personality_fn{ llvm_module->getOrInsertFunction("__gxx_personality_v0",
                                                            personality_fn_type) };
      current_fn->setPersonalityFn(llvm::cast<llvm::Function>(personality_fn.getCallee()));
    }

    auto const is_return(expr->position == expression_position::tail);
    auto const has_finally{ expr->finally_body.is_some() };
    auto const has_catch{ expr->catch_body.is_some() };

    /* unwind_flag_slot: An alloca for a boolean (i1). This flag is set to true if the 'finally'
     * block is being entered as part of an exception unwinding process (e.g., from a landing pad).
     * It's false if 'finally' is entered after normal completion of the try or catch block.
     * This controls whether to resume unwinding or continue normally after the "finally" block. */
    llvm::AllocaInst *unwind_flag_slot{};

    /* exception_slot: An alloca for a pointer. When unwinding_flag_slot is true, this slot holds
     * the exception object (typically an i8* or a struct pointer) that was caught by the landing
     * pad. This pointer is needed if the exception needs to be resumed or rethrown after the
     * 'finally' block. */
    llvm::AllocaInst *exception_slot{};

    /* result_slot: An alloca for a pointer (object_ref). This slot holds the llvm::Value*
     * that represents the result of the (try ...) expression.
     *
     * - If the 'try' block completes without an exception, its result is stored here.
     * - If an exception is caught and handled by a 'catch' block, the result of the
     *   'catch' block is stored here.
     *
     * This value is then loaded in the continuation block ('cont_bb') after any
     * 'finally' block has executed. Because control flow always passes through the 'finally'
     * block on normal exits (if a finally exists), we can't directly use a PHI node in
     * 'cont_bb' with predecessors from the end of 'try' and 'catch'. This slot acts as a
     * temporary variable to hold the result before entering 'finally'. */
    llvm::AllocaInst *result_slot{ entry_builder.CreateAlloca(ptr_ty, nullptr, "try.result.slot") };
    llvm::BasicBlock *finally_bb{};
    llvm::BasicBlock *unwind_action_bb{};
    ctx->builder->CreateStore(gen_global(jank_nil()), result_slot);

    if(has_finally)
    {
      unwind_flag_slot
        = entry_builder.CreateAlloca(ctx->builder->getInt1Ty(), nullptr, "unwind.flag.slot");
      exception_slot = entry_builder.CreateAlloca(ptr_ty, nullptr, "exception.slot");
      finally_bb = llvm::BasicBlock::Create(*llvm_ctx, "finally");
      unwind_action_bb = llvm::BasicBlock::Create(*llvm_ctx, "unwind.action");
    }
    auto const cont_bb{ llvm::BasicBlock::Create(*llvm_ctx, "try.cont") };
    auto const lpad_bb{ llvm::BasicBlock::Create(*llvm_ctx, "lpad", current_fn) };

    llvm::BasicBlock *catch_body_bb{};
    if(has_catch)
    {
      catch_body_bb = llvm::BasicBlock::Create(*llvm_ctx, "catch.body");
    }

    lpad_and_catch_body_stack.emplace_back(lpad_bb, catch_body_bb);
    util::scope_exit const pop_landing_pad{ [this]() { lpad_and_catch_body_stack.pop_back(); } };

    /* --- Try block --- */
    auto const original_try_pos{ expr->body->position };

    /* We put the try body into the value position so that no return is generated, which allows
     * us to continue onto the finally block, if we have one. */
    expr->body->propagate_position(expression_position::value);
    auto const try_val{ gen(expr->body, arity) };
    expr->body->propagate_position(original_try_pos);

    /* Handles the normal completion of the 'try' block.
     * If code generation for the 'try' body produces a value (try_val is not null)
     * and the current basic block doesn't already have a terminator (e.g., from a return
     * or throw within the try body itself), this block adds the necessary instructions.
     *
     * 1. Store Result: The result of the 'try' block (try_val) is stored into the
     *    'result_slot' to be potentially used after the 'finally' block.
     * 2. Branch to finally or continuation:
     *    - If a 'finally' block exists ('has_finally' is true), it prepares for
     *      entering the finally block normally. This involves setting the 'unwind_flag_slot'
     *      to false (signifying not unwinding from an exception) and creating an
     *      unconditional branch to 'finally_bb'.
     *    - If there's no 'finally' block, it branches directly to the continuation
     *      block 'cont_bb', as the try-catch-finally construct is complete. */
    if(try_val && !ctx->builder->GetInsertBlock()->getTerminator())
    {
      ctx->builder->CreateStore(try_val, result_slot);
      if(has_finally)
      {
        ctx->builder->CreateStore(ctx->builder->getFalse(), unwind_flag_slot);
        ctx->builder->CreateBr(finally_bb);
      }
      else
      {
        ctx->builder->CreateBr(cont_bb);
      }
    }

    /* --- Landing Pad & Catch/Resume Logic ---
     * We are now about to generate code for the landing pad (lpad_bb), which catches
     * exceptions thrown from the preceding 'try' block.
     *
     * IMPORTANT: Exceptions thrown from *within* the 'catch' or 'finally' clauses
     * associated with THIS try-catch-finally statement should NOT be caught by this same
     * landing pad (lpad_bb). Instead, they should be handled by any outer exception
     * handlers or propagate up.
     *
     * To achieve this, we pop the current (lpad_bb, catch_body_bb) pair from the
     * 'lpad_and_catch_body_stack'. This stack is used by 'CreateInvoke' to determine
     * the unwind destination. By popping, any 'invoke' calls within the catch/finally
     * code will use the *next* landing pad on the stack (if any), belonging to an
     * enclosing 'try' statement. */
    lpad_and_catch_body_stack.pop_back();
    ctx->builder->SetInsertPoint(lpad_bb);
    auto const i32_ty{ ctx->builder->getInt32Ty() };
    auto const lpad_ty{ llvm::StructType::get(*llvm_ctx, { ptr_ty, i32_ty }) };
    auto const landing_pad{ ctx->builder->CreateLandingPad(lpad_ty, 1) };

    if(has_finally)
    {
      /* Mark the landing pad as a "cleanup" landing pad.
       * A cleanup landing pad indicates that there is cleanup code (the 'finally' block)
       * that MUST be executed regardless of whether the current exception is caught by
       * any of the clauses in this landing pad instruction or not.
       *
       * Effect: When an exception is caught by this landing_pad:
       * 1. The personality function is called.
       * 2. If the exception type matches any of the 'addClause' types, control might
       *    go to the catch block.
       * 3. CRUCIALLY, because setCleanup(true) is set, even if the exception type does
       *    NOT match any clause, or after a catch block finishes, the control flow
       *    is structured to eventually execute the cleanup code associated with this
       *    unwind path (which we've designed to be the 'finally_bb').
       *
       * In essence, 'setCleanup(true)' ensures that the unwinding process will not
       * bypass the 'finally' block's execution. After the 'finally' block, the exception
       * handling might continue (e.g., by resuming the unwind if the exception wasn't
       * fully handled). */
      landing_pad->setCleanup(true);
    }

    auto exception_ptr{ ctx->builder->CreateExtractValue(landing_pad, 0, "ex.ptr") };
    if(has_catch)
    {
      /* To make the landing pad catch specific types of exceptions, we need to add clauses.
       * Each clause represents a type of exception this landing pad can handle.
       *
       * We need a reference to the type information for the exception type we want to catch.
       * The Itanium C++ ABI exception handling mechanism uses type info globals.
       * The exact type of the global doesn't matter as much as its address, which is used
       * by the personality function to identify the exception type.
       *
       * When an exception is thrown, the personality function compares the thrown
       * exception's type info with the clauses added to the landing pads in the call stack.
       * If a match is found, control is transferred to this landing pad. */
      auto const catch_type{ expr->catch_body.unwrap().type };
      auto const exception_rtti{ Cpp::MangleRTTI(catch_type) };

      /* macOS requires explicit registration of RTTI symbols. */
      if constexpr(jtl::current_platform == jtl::platform::macos_like)
      {
        static native_set<jtl::immutable_string> rtti_syms;
        if(!rtti_syms.contains(exception_rtti))
        {
          /* We need to register this RTTI right now, for the JIT. */
          cpp_util::register_rtti(catch_type);
          rtti_syms.emplace(exception_rtti);
        }

        /* We also need to surface this RTTI upward, to the module level, so it
         * can end up in the generated object file. */
        auto const callable{
          Cpp::MakeRTTICallable(catch_type, exception_rtti, unique_munged_string())
        };
        global_rtti.emplace(exception_rtti, callable);
      }

      auto const exception_rtti_global{ llvm_module->getOrInsertGlobal(exception_rtti,
                                                                       ctx->builder->getPtrTy()) };

      landing_pad->addClause(exception_rtti_global);

      /* Setup for handling exceptions that might be thrown FROM WITHIN the catch block itself.
       * We need to ensure that if an exception occurs inside the 'catch' body,
       * any 'finally' block is still executed. This is achieved by having a dedicated
       * landing pad ('cleanup_lpad_bb') for the 'catch' body's scope. */
      llvm::BasicBlock *cleanup_lpad_bb{};
      if(has_finally)
      {
        /* To make any potentially throwing function calls (which will be generated as
         * llvm::InvokeInst) within the *catch body* unwind to our 'cleanup_lpad_bb',
         * we must push 'cleanup_lpad_bb' onto the 'lpad_and_catch_body_stack'.
         * The code generation for 'invoke' uses the top of this stack as the
         * unwind destination. */
        cleanup_lpad_bb = llvm::BasicBlock::Create(*llvm_ctx, "cleanup.lpad", current_fn);
        lpad_and_catch_body_stack.emplace_back(cleanup_lpad_bb, nullptr);
      }
      util::scope_exit const pop_cleanup_lpad{ [this, has_finally]() {
        if(has_finally)
        {
          lpad_and_catch_body_stack.pop_back();
        }
      } };

      ctx->builder->CreateBr(catch_body_bb);
      current_fn->insert(current_fn->end(), catch_body_bb);
      ctx->builder->SetInsertPoint(catch_body_bb);

      auto const &[catch_sym, _, catch_body]{ expr->catch_body.unwrap() };
      auto old_locals(locals);
      auto const begin_catch_fn{
        llvm_module->getOrInsertFunction("__cxa_begin_catch", ptr_ty, ptr_ty)
      };
      auto const caught_ptr{ ctx->builder->CreateCall(begin_catch_fn, { exception_ptr }) };
      locals[catch_sym] = ctx->builder->CreateLoad(ptr_ty, caught_ptr, "ex.val");

      auto const original_catch_pos{ catch_body->position };
      catch_body->propagate_position(expression_position::value);
      util::scope_exit const restore_catch_pos{ [&]() {
        catch_body->propagate_position(original_catch_pos);
      } };
      auto catch_val{ gen(catch_body, arity) };

      auto const end_catch_fn{ llvm_module->getOrInsertFunction("__cxa_end_catch",
                                                                ctx->builder->getVoidTy()) };
      ctx->builder->CreateCall(end_catch_fn, {});
      locals = std::move(old_locals);

      if(!ctx->builder->GetInsertBlock()->getTerminator())
      {
        if(!catch_val)
        {
          catch_val = gen_global(jank_nil());
        }
        ctx->builder->CreateStore(catch_val, result_slot);
        if(has_finally)
        {
          ctx->builder->CreateStore(ctx->builder->getFalse(), unwind_flag_slot);
          ctx->builder->CreateBr(finally_bb);
        }
        else
        {
          ctx->builder->CreateBr(cont_bb);
        }
      }

      if(has_finally)
      {
        /* This block populates 'cleanup_lpad_bb', which acts as the landing pad
        * for any exception thrown *within* the execution of the 'catch' block body.
        * Its primary purpose is to ensure the 'finally' block is executed
        * even if the catch handler itself throws. */
        ctx->builder->SetInsertPoint(cleanup_lpad_bb);

        /* Create the landing pad instruction for the catch block's cleanup.
         * It takes no clauses because it's not trying to "catch" and handle
         * the exception in the sense of stopping propagation, but rather to
         * perform the necessary cleanups. */
        auto const cleanup_lpad{ ctx->builder->CreateLandingPad(lpad_ty, 0) };
        cleanup_lpad->setCleanup(true);

        /* Extract the pointer to the new exception object that was caught. And store the pointer
         * to the exception object that was caught *inside the catch block*.
         * This exception object will be needed in 'unwind_action_bb' after the 'finally'
         * block runs to resume the stack unwinding process with this new exception. */
        auto cleanup_ex_ptr{ ctx->builder->CreateExtractValue(cleanup_lpad, 0, "cleanup.ex.ptr") };
        ctx->builder->CreateStore(cleanup_ex_ptr, exception_slot);

        /* Set the unwind flag to TRUE. We are inside a landing pad (cleanup_lpad_bb),
         * which is only ever entered as a result of an exception being thrown.
         * Therefore, we are definitely in an exception unwinding state. This flag
         * signals to the code in 'finally_bb' that it should branch to
         * 'unwind_action_bb' after completing the 'finally' logic, rather than
         * continuing to 'cont_bb' as would happen in a normal execution flow. */
        ctx->builder->CreateStore(ctx->builder->getTrue(), unwind_flag_slot);
        ctx->builder->CreateBr(finally_bb);
      }
    }
    else
    {
      /* No catch, must have 'finally'. */
      ctx->builder->CreateStore(exception_ptr, exception_slot);
      ctx->builder->CreateStore(ctx->builder->getTrue(), unwind_flag_slot);
      ctx->builder->CreateBr(finally_bb);
    }

    /* --- Finally block --- */
    if(has_finally)
    {
      current_fn->insert(current_fn->end(), finally_bb);
      ctx->builder->SetInsertPoint(finally_bb);

      gen(expr->finally_body.unwrap(), arity);

      if(!ctx->builder->GetInsertBlock()->getTerminator())
      {
        auto unwind_flag = ctx->builder->CreateLoad(ctx->builder->getInt1Ty(), unwind_flag_slot);
        ctx->builder->CreateCondBr(unwind_flag, unwind_action_bb, cont_bb);
      }

      /* --- Unwind Action block ---
       * This block is entered from 'finally_bb' ONLY when the 'finally' block
       * was executed as part of an exception unwinding process (i.e., unwind_flag_slot was true).
       * The purpose of this block is to continue the exception propagation after
       * the cleanup code in 'finally_bb' has run. The exception object to be
       * propagated was saved in 'exception_slot'. */
      current_fn->insert(current_fn->end(), unwind_action_bb);
      ctx->builder->SetInsertPoint(unwind_action_bb);
      auto current_ex = ctx->builder->CreateLoad(ptr_ty, exception_slot);

      /* Determine how to propagate the exception:
       * - If 'lpad_and_catch_body_stack' is not empty, it means there's an enclosing
       *   'try' block within the *same* function. We should "rethrow" the exception
       *   in a way that it can be caught by the landing pad of that outer 'try' block.
       * - If the stack is empty, there are no more exception handlers within this
       *   function to transfer control to, so we must resume the standard stack unwinding
       *   process, allowing handlers in caller functions to catch the exception. */
      if(!lpad_and_catch_body_stack.empty())
      {
        /* Propagate to an outer landing pad in the same function.
         * To ensure the outer catch receives the correct user exception object,
         * we first need to extract it using the C++ ABI helper functions. */
        auto exception_ptr_reloaded{ current_ex };
        auto const begin_catch_fn{
          llvm_module->getOrInsertFunction("__cxa_begin_catch", ptr_ty, ptr_ty)
        };
        auto const caught_ptr{ ctx->builder->CreateCall(begin_catch_fn,
                                                        { exception_ptr_reloaded }) };
        auto const ex_val_ptr{ ctx->builder->CreateLoad(ptr_ty, caught_ptr, "ex.val") };
        auto const end_catch_fn{ llvm_module->getOrInsertFunction("__cxa_end_catch",
                                                                  ctx->builder->getVoidTy()) };
        ctx->builder->CreateCall(end_catch_fn, {});

        /* Now, rethrow the *user exception object* (ex_val_ptr) using jank_throw.
         * This call is wrapped in CreateInvoke, with the outer try's landing pad
         * as the unwind destination. */
        auto const unreachable_dest{
          llvm::BasicBlock::Create(*llvm_ctx, "unreachable.throw", current_fn)
        };
        auto const fn_type{
          llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false)
        };
        auto function_callee{ llvm_module->getOrInsertFunction("jank_throw", fn_type) };
        llvm::cast<llvm::Function>(function_callee.getCallee())->setDoesNotReturn();

        ctx->builder->CreateInvoke(function_callee,
                                   unreachable_dest,
                                   lpad_and_catch_body_stack.back().lpad_bb,
                                   { ex_val_ptr });
        ctx->builder->SetInsertPoint(unreachable_dest);
        ctx->builder->CreateUnreachable();
      }
      else
      {
        /* No outer 'try' handlers within this function's scope. We need to resume
         * the standard stack unwinding process. This allows the exception to propagate
         * up the call stack to potentially be caught by handlers in caller functions.
         * The 'llvm.resume' instruction is used for this purpose.
         *
         * We need to reconstruct the two-element struct { i8*, i32 } that 'llvm.resume'
         * expects. This struct is the same type as what a 'landing pad' instruction returns.
         * The first element is the exception pointer, and the second is a selector value. */
        auto lpad_val{
          ctx->builder->CreateInsertValue(llvm::UndefValue::get(lpad_ty), current_ex, 0)
        };
        lpad_val = ctx->builder->CreateInsertValue(lpad_val, ctx->builder->getInt32(0), 1);
        ctx->builder->CreateResume(lpad_val);
      }
    }

    /* We pushed the landing pad for our `try` block. It has now been popped, before
     * generating catch/finally. We must push it back on so that the scope_exit
     * guard at the top can correctly pop it later, restoring the stack for the rest
     * of this function's codegen. */
    lpad_and_catch_body_stack.emplace_back(lpad_bb, catch_body_bb);

    /* --- Continuation block --- */
    current_fn->insert(current_fn->end(), cont_bb);
    ctx->builder->SetInsertPoint(cont_bb);
    auto final_val = ctx->builder->CreateLoad(ptr_ty, result_slot);

    if(is_return)
    {
      gen_ret(final_val);
    }
    return final_val;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::case_ref const expr, expr::function_arity const &arity)
  {
    auto const current_fn(ctx->builder->GetInsertBlock()->getParent());
    auto const position{ expr->position };
    auto const value(gen(expr->value_expr, arity));
    auto const is_return{ position == expression_position::tail };
    auto const integer_fn_type(llvm::FunctionType::get(
      ctx->builder->getInt64Ty(),
      { ctx->builder->getPtrTy(), ctx->builder->getInt64Ty(), ctx->builder->getInt64Ty() },
      false));
    auto const fn(
      llvm_module->getOrInsertFunction("jank_shift_mask_case_integer", integer_fn_type));
    llvm::SmallVector<llvm::Value *, 3> const args{
      value,
      llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), expr->shift),
      llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), expr->mask)
    };
    auto const call(ctx->builder->CreateCall(fn, args));
    auto const switch_val(ctx->builder->CreateIntCast(call, ctx->builder->getInt64Ty(), true));
    auto const default_block{ llvm::BasicBlock::Create(*llvm_ctx, "default", current_fn) };
    auto const switch_{ ctx->builder->CreateSwitch(switch_val, default_block, expr->keys.size()) };
    auto const merge_block{ is_return ? nullptr
                                      : llvm::BasicBlock::Create(*llvm_ctx, "merge", current_fn) };

    ctx->builder->SetInsertPoint(default_block);
    auto const default_val{ gen(expr->default_expr, arity) };
    if(!is_return)
    {
      ctx->builder->CreateBr(merge_block);
    }
    auto const default_block_exit{ ctx->builder->GetInsertBlock() };

    llvm::SmallVector<llvm::BasicBlock *> case_blocks;
    llvm::SmallVector<llvm::Value *> case_values;
    for(usize block_counter{}; block_counter < expr->keys.size(); ++block_counter)
    {
      auto const block_name{ util::format("case_{}", block_counter) };
      auto const block{ llvm::BasicBlock::Create(*llvm_ctx, block_name.c_str(), current_fn) };
      switch_->addCase(
        llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), expr->keys[block_counter]),
        block);

      ctx->builder->SetInsertPoint(block);
      auto const case_val{ gen(expr->exprs[block_counter], arity) };
      case_values.push_back(case_val);
      if(!is_return)
      {
        ctx->builder->CreateBr(merge_block);
      }
      case_blocks.push_back(ctx->builder->GetInsertBlock());
    }

    if(!is_return)
    {
      ctx->builder->SetInsertPoint(merge_block);
      auto const phi{
        ctx->builder->CreatePHI(ctx->builder->getPtrTy(), expr->keys.size() + 1, "switch_tmp")
      };
      phi->addIncoming(default_val, default_block_exit);
      for(usize i{}; i < case_blocks.size(); ++i)
      {
        phi->addIncoming(case_values[i], case_blocks[i]);
      }
      return phi;
    }
    return nullptr;
  }

  /* NOLINTNEXTLINE(readability-make-member-function-const): Affects overload resolution. */
  llvm::Value *llvm_processor::impl::gen(expr::cpp_raw_ref const expr, expr::function_arity const &)
  {
    auto parse_res{ __rt_ctx->jit_prc.interpreter->Parse(expr->code.c_str()) };
    if(!parse_res)
    {
      throw std::runtime_error{ "Unable to parse 'cpp/raw' expression." };
    }
    link_module(*ctx, parse_res->TheModule.get());

    auto const ret{ gen_global(jank_nil()) };
    if(expr->position == expression_position::tail)
    {
      return gen_ret(ret);
    }
    return ret;
  }

  /* NOLINTNEXTLINE(readability-make-member-function-const): Affects overload resolution. */
  llvm::Value *llvm_processor::impl::gen(expr::cpp_type_ref const, expr::function_arity const &)
  {
    throw std::runtime_error{ "cpp_type has no codegen" };
  }

  llvm::Value *
  /* NOLINTNEXTLINE(readability-make-member-function-const): Can't be const, due to overload resolution ambiguities. */
  llvm_processor::impl::gen(expr::cpp_value_ref const expr, expr::function_arity const &)
  {
    if(expr->val_kind == expr::cpp_value::value_kind::null)
    {
      auto const alloc{ ctx->builder->CreateAlloca(
        ctx->builder->getPtrTy(),
        llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
      auto const null{ llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()) };
      ctx->builder->CreateStore(null, alloc);
      if(expr->position == expression_position::tail)
      {
        return gen_ret(alloc);
      }
      return alloc;
    }
    if(expr->val_kind == expr::cpp_value::value_kind::bool_true
       || expr->val_kind == expr::cpp_value::value_kind::bool_false)
    {
      auto const val{ expr->val_kind == expr::cpp_value::value_kind::bool_true };
      auto const alloc{ ctx->builder->CreateAlloca(
        ctx->builder->getInt8Ty(),
        llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
      auto const ir_val{ llvm::ConstantInt::getSigned(ctx->builder->getInt8Ty(), val) };
      ctx->builder->CreateStore(ir_val, alloc);
      if(expr->position == expression_position::tail)
      {
        return gen_ret(alloc);
      }
      return alloc;
    }
    if(expr->val_kind == expr::cpp_value::value_kind::enum_constant)
    {
      auto const val{ Cpp::GetEnumConstantValue(expr->scope) };
      auto const int_type{ llvm_builtin_type(
        *ctx,
        llvm_ctx,
        Cpp::GetIntegerTypeFromEnumType(Cpp::GetNonReferenceType(expr->type))) };
      auto const alloc{
        ctx->builder->CreateAlloca(int_type, llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1))
      };
      auto const ir_val{ llvm::ConstantInt::getSigned(int_type, static_cast<int64_t>(val)) };
      ctx->builder->CreateStore(ir_val, alloc);
      if(expr->position == expression_position::tail)
      {
        return gen_ret(alloc);
      }
      return alloc;
    }

    auto const callable{
      Cpp::IsFunctionPointerType(expr->type)
        /* We pass the type and the scope in here so that unresolved template
                            * scopes can be turned into the correct specialization which matches
                            * the type we have. */
        ? Cpp::MakeFunctionValueAotCallable(expr->scope, expr->type, unique_munged_string())
        : Cpp::MakeAotCallable(expr->scope, unique_munged_string())
    };
    jank_debug_assert(callable);
    link_module(*ctx, reinterpret_cast<llvm::Module *>(callable.getModule()));

    auto const alloc{ alloc_type(*ctx, llvm_ctx, expr->type) };
    auto const fn(llvm_module->getFunction(callable.getName()));
    llvm::SmallVector<llvm::Value *, 4> const args{
      llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()),
      llvm::ConstantInt::getSigned(ctx->builder->getInt32Ty(), 0),
      llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()),
      alloc
    };
    ctx->builder->CreateCall(fn, args);

    if(expr->position == expression_position::tail)
    {
      return gen_ret(alloc);
    }

    return alloc;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::cpp_cast_ref const expr, expr::function_arity const &arity)
  {
    auto const value{ gen(expr->value_expr, arity) };
    auto converted{ convert_object(*ctx,
                                   llvm_ctx,
                                   llvm_module,
                                   expr->policy,
                                   cpp_util::expression_type(expr->value_expr),
                                   expr->type,
                                   expr->conversion_type,
                                   value) };

    if(expr->position == expression_position::tail)
    {
      return gen_ret(load_if_needed(ctx, converted));
    }

    return converted;
  }

  llvm::Value *llvm_processor::impl::gen_aot_call(Cpp::AotCall const &call,
                                                  jtl::ptr<void> const fn,
                                                  jtl::ptr<void> const expr_type,
                                                  jtl::immutable_string const &name,
                                                  native_vector<expression_ref> const &arg_exprs,
                                                  expression_position const position,
                                                  expression_kind const kind,
                                                  expr::function_arity const &arity)
  {
    jank_debug_assert(expr_type);
    auto const is_void{ Cpp::IsVoid(expr_type) };
    llvm::Value *ret_alloc{};
    if(!is_void)
    {
      ret_alloc = alloc_type(*ctx, llvm_ctx, expr_type, util::format("{}.res", name));
    }

    return gen_aot_call(call, ret_alloc, fn, expr_type, name, arg_exprs, position, kind, arity);
  }

  llvm::Value *llvm_processor::impl::gen_aot_call(Cpp::AotCall const &call,
                                                  llvm::Value *ret_alloc,
                                                  jtl::ptr<void> const fn,
                                                  jtl::ptr<void> const expr_type,
                                                  jtl::immutable_string const &name,
                                                  native_vector<expression_ref> const &arg_exprs,
                                                  expression_position const position,
                                                  expression_kind const kind,
                                                  expr::function_arity const &arity)
  {
    link_module(*ctx, reinterpret_cast<llvm::Module *>(call.getModule()));

    jank_debug_assert(expr_type);
    auto const is_void{ Cpp::IsVoid(expr_type) };

    /* For member function calls, we steal the first argument and use it as
     * the invoking object. Otherwise, we pass null as the invoking object. */
    auto const requires_this_obj{ kind == expression_kind::cpp_member_call
                                  /* TODO: Not required if the member is static. */
                                  || kind == expression_kind::cpp_member_access };
    if(requires_this_obj)
    {
      jank_debug_assert(!arg_exprs.empty());
    }
    llvm::Value *this_obj{ llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()) };
    auto const member_offset{ requires_this_obj ? 1 : 0 };
    auto const arg_count{ arg_exprs.size() - member_offset };
    auto const args_array_type{ llvm::ArrayType::get(ctx->builder->getPtrTy(), arg_count) };
    auto const args_array{ arg_count == 0
                             ? static_cast<llvm::Value *>(
                                 llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()))
                             : static_cast<llvm::Value *>(ctx->builder->CreateAlloca(
                                 args_array_type,
                                 nullptr,
                                 util::format("{}.args", name).c_str())) };

    for(usize i{}; i < arg_exprs.size(); ++i)
    {
      auto arg_handle{ gen(arg_exprs[i], arity) };
      auto const arg_type{ cpp_util::expression_type(arg_exprs[i]) };
      auto const is_arg_ref{ Cpp::IsReferenceType(arg_type)
                             && !(Cpp::IsPointerType(Cpp::GetNonReferenceType(arg_type))
                                  || Cpp::IsArrayType(Cpp::GetNonReferenceType(arg_type))) };
      auto const is_arg_ptr{ Cpp::IsPointerType(arg_type) || Cpp::IsArrayType(arg_type)
                             || cpp_util::is_any_object(arg_type) };
      auto const is_arg_indirect{ is_arg_ref || is_arg_ptr };

      if(i == 0 && requires_this_obj)
      {
        this_obj = arg_handle;
        if(is_arg_indirect && llvm::isa<llvm::AllocaInst>(this_obj))
        {
          this_obj = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), arg_handle);
        }
        continue;
      }

      auto const is_arg_untyped_obj{ cpp_util::is_untyped_object(arg_type) };
      jtl::ptr<void> param_type{ fn ? Cpp::GetFunctionArgType(fn, i - member_offset) : nullptr };
      /* If our function is variadic, we won't have a param type for each variadic
       * param. Instead, we use the arg type. */
      if(!param_type)
      {
        /* If we're constructing a builtin type, we don't have a ctor fn. We know the
         * param type we need, though. */
        if(kind == expression_kind::cpp_constructor_call)
        {
          param_type = expr_type.data;
        }
        else if(kind == expression_kind::cpp_builtin_operator_call)
        {
          param_type = Cpp::GetNonReferenceType(arg_type);
        }
        else
        {
          param_type = arg_type;
        }
      }
      auto const is_param_ptr{ Cpp::IsPointerType(param_type) || Cpp::IsArrayType(param_type)
                               || cpp_util::is_any_object(param_type) };
      auto const is_param_indirect{ Cpp::IsReferenceType(param_type) || is_param_ptr };
      //util::println(
      //  "gen_aot_call arg {}, arg type {} {} (indirect {}), param type {} {} (indirect {}), "
      //  "implicitly convertible {}",
      //  i,
      //  arg_type,
      //  Cpp::GetTypeAsString(arg_type),
      //  is_arg_indirect,
      //  param_type,
      //  Cpp::GetTypeAsString(param_type),
      //  is_param_indirect,
      //  Cpp::IsImplicitlyConvertible(arg_type, param_type));

      /* TODO: Is this needed? I thought our casts were explicit in the AST. */
      if(is_arg_untyped_obj
         && (cpp_util::is_primitive(param_type)
             || !Cpp::IsImplicitlyConvertible(arg_type, param_type)))
      {
        arg_handle = convert_object(*ctx,
                                    llvm_ctx,
                                    llvm_module,
                                    conversion_policy::from_object,
                                    arg_type,
                                    param_type,
                                    param_type,
                                    arg_handle);
      }
      else if((is_arg_ref && llvm::isa<llvm::AllocaInst>(arg_handle))
              || (is_arg_ptr && !is_param_indirect && llvm::isa<llvm::AllocaInst>(arg_handle)))
      {
        arg_handle = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), arg_handle);
      }
      /* TODO: This is borked, but maybe close to what we want. Figure out the exact logic. */
      else if(is_arg_ptr && !llvm::isa<llvm::AllocaInst>(arg_handle) && is_param_ptr)
      {
        auto const alloc{ ctx->builder->CreateAlloca(
          ctx->builder->getPtrTy(),
          llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
        ctx->builder->CreateStore(arg_handle, alloc);
        arg_handle = alloc;
      }
      //else if(!is_arg_ref && is_param_indirect)
      //{
      //  /* TODO: Nothing to do here? */
      //}

      auto const arg_ptr{ ctx->builder->CreateInBoundsGEP(
        args_array_type,
        args_array,
        { ctx->builder->getInt32(0), ctx->builder->getInt32(i - member_offset) },
        util::format("{}.args[{}]", name, i - member_offset).c_str()) };
      ctx->builder->CreateStore(arg_handle, arg_ptr);
    }

    auto const sret{ is_void ? static_cast<llvm::Value *>(
                                 llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()))
                             : ret_alloc };
    auto const target_fn(llvm_module->getFunction(call.getName()));
    llvm::SmallVector<llvm::Value *, 4> const ctor_args{
      this_obj,
      llvm::ConstantInt::getSigned(ctx->builder->getInt32Ty(), static_cast<i64>(arg_count)),
      args_array,
      sret
    };
    ctx->builder->CreateCall(target_fn, ctor_args);

    if(position == expression_position::tail)
    {
      if(is_void)
      {
        return gen_ret(gen_global(jank_nil()));
      }

      auto const ret_load{ ctx->builder->CreateLoad(ctx->builder->getPtrTy(), ret_alloc, "ret") };
      return gen_ret(ret_load);
    }

    if(is_void)
    {
      return gen_global(jank_nil());
    }
    return ret_alloc;
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::cpp_call_ref const expr, expr::function_arity const &arity)
  {
    /* This is the second step for `cpp/value` literals. */
    if(target == compilation_target::module && !expr->function_code.empty())
    {
      auto parse_res{ runtime::__rt_ctx->jit_prc.interpreter->Parse(expr->function_code.c_str()) };
      if(!parse_res)
      {
        throw std::runtime_error{ "Unable to parse C++ literal." };
      }
      link_module(*ctx, parse_res->TheModule.get());
    }

    std::vector<Cpp::TCppType_t> arg_types;
    arg_types.reserve(expr->arg_exprs.size());
    for(auto const arg_expr : expr->arg_exprs)
    {
      arg_types.emplace_back(cpp_util::expression_type(arg_expr));
    }

    if(expr->source_expr->kind == expression_kind::cpp_value)
    {
      auto const source{ llvm::cast<expr::cpp_value>(expr->source_expr.data) };
      return gen_aot_call(Cpp::MakeAotCallable(source->scope, arg_types, unique_munged_string()),
                          source->scope,
                          expr->type,
                          Cpp::GetName(source->scope),
                          expr->arg_exprs,
                          expr->position,
                          expr->kind,
                          arity);
    }
    else
    {
      auto const source_type{ cpp_util::expression_type(expr->source_expr) };
      auto arg_exprs{ expr->arg_exprs };
      arg_exprs.insert(arg_exprs.begin(), expr->source_expr);
      return gen_aot_call(Cpp::MakeApplyCallable(source_type, arg_types, unique_munged_string()),
                          nullptr,
                          expr->type,
                          "call",
                          jtl::move(arg_exprs),
                          expr->position,
                          expr->kind,
                          arity);
    }
  }

  llvm::Value *llvm_processor::impl::gen(expr::cpp_constructor_call_ref const expr,
                                         expr::function_arity const &arity,
                                         llvm::Value * const alloc)
  {
    auto const is_primitive{ cpp_util::is_primitive(expr->type) };
    Cpp::AotCall ctor_fn_callable;
    if(is_primitive)
    {
      if(expr->arg_exprs.empty())
      {
        /* TODO: We should just be able to alloc the type here and zero the memory.
         * We can save ourselves the time of JIT compiling more C++ and make the IR easier
         * to optimize. */
        ctor_fn_callable
          = Cpp::MakeBuiltinConstructorAotCallable(expr->type, unique_munged_string());
      }
      else
      {
        jank_debug_assert(expr->arg_exprs.size() == 1);
        auto const arg_type{ cpp_util::expression_type(expr->arg_exprs[0]) };
        auto const needs_conversion{ !Cpp::IsConstructible(expr->type, arg_type) };

        ctor_fn_callable
          = Cpp::MakeBuiltinConstructorAotCallable(expr->type,
                                                   needs_conversion ? expr->type : arg_type,
                                                   unique_munged_string());
      }
    }
    else if(expr->is_aggregate)
    {
      std::vector<Cpp::TemplateArgInfo> arg_types;
      for(auto const &arg_expr : expr->arg_exprs)
      {
        arg_types.emplace_back(cpp_util::expression_type(arg_expr));
      }
      ctor_fn_callable = Cpp::MakeAggregateInitializationAotCallable(expr->type,
                                                                     arg_types,
                                                                     unique_munged_string());
    }
    else
    {
      jank_debug_assert(expr->fn);
      ctor_fn_callable = Cpp::MakeAotCallable(expr->fn, unique_munged_string());
    }
    jank_debug_assert(ctor_fn_callable);

    return gen_aot_call(ctor_fn_callable,
                        alloc,
                        expr->fn,
                        expr->type,
                        Cpp::GetTypeAsString(expr->type),
                        expr->arg_exprs,
                        expr->position,
                        expr->kind,
                        arity);
  }

  llvm::Value *llvm_processor::impl::gen(expr::cpp_constructor_call_ref const expr,
                                         expr::function_arity const &arity)
  {
    return gen(expr, arity, alloc_type(*ctx, llvm_ctx, expr->type));
  }

  llvm::Value *
  llvm_processor::impl::gen(expr::cpp_member_call_ref const expr, expr::function_arity const &arity)
  {
    return gen_aot_call(Cpp::MakeAotCallable(expr->fn, unique_munged_string()),
                        expr->fn,
                        cpp_util::expression_type(expr),
                        Cpp::GetName(expr->fn),
                        expr->arg_exprs,
                        expr->position,
                        expr->kind,
                        arity);
  }

  llvm::Value *llvm_processor::impl::gen(expr::cpp_member_access_ref const expr,
                                         expr::function_arity const &arity)
  {
    return gen_aot_call(Cpp::MakeAotCallable(expr->scope, unique_munged_string()),
                        nullptr,
                        expr->type,
                        Cpp::GetName(expr->scope),
                        { expr->obj_expr },
                        expr->position,
                        expr->kind,
                        arity);
  }

  llvm::Value *llvm_processor::impl::gen(analyze::expr::cpp_builtin_operator_call_ref const expr,
                                         analyze::expr::function_arity const &arity)
  {
    /* If we're doing a deref, there are a couple of special cases. If our output is a
     * reference, that means we're dereferencing a pointer to a reference, which doesn't
     * change the underlying codegen value, so we don't do any deref. If our output is a
     * pointer, that means we're dereferencing a pointer to pointer, so we just short-circuit
     * the whole CppInterOp dance and do a load. */
    if(expr->op == Cpp::OP_Star && expr->arg_exprs.size() == 1)
    {
      auto const arg_type{ cpp_util::expression_type(expr->arg_exprs[0]) };
      auto const arg_handle{ gen(expr->arg_exprs[0], arity) };
      auto const arg_ptr_count{ pointer_count(arg_type) };
      auto const target_ptr_count{ pointer_count(expr->type) };
      auto const ptr_diff{ arg_ptr_count - target_ptr_count };

      if(ptr_diff == 0)
      {
        return arg_handle;
      }

      auto const alloc{ ctx->builder->CreateAlloca(
        ctx->builder->getPtrTy(),
        llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
      auto value{ arg_handle };
      for(int i{}; i < ptr_diff + llvm::isa<llvm::AllocaInst>(arg_handle); ++i)
      {
        value = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), value);
      }
      ctx->builder->CreateStore(value, alloc);
      return alloc;
    }
    else if(expr->op == Cpp::OP_Amp && expr->arg_exprs.size() == 1)
    {
      /* Getting the address of a reference should just yield the same reference, at the IR
       * level. This is the case for two reasons.
       *
       * 1. References don't have their own storage, in C++.
       * 2. References are implemented as pointers behind the scenes.
       */
      if(Cpp::IsReferenceType(cpp_util::expression_type(expr->arg_exprs[0])))
      {
        return gen(expr->arg_exprs[0], arity);
      }

      auto const alloc{ ctx->builder->CreateAlloca(
        ctx->builder->getPtrTy(),
        llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };

      auto const value{ gen(expr->arg_exprs[0], arity) };
      ctx->builder->CreateStore(value, alloc);
      return alloc;
    }

    std::vector<Cpp::TemplateArgInfo> arg_types;
    for(auto const e : expr->arg_exprs)
    {
      arg_types.emplace_back(cpp_util::expression_type(e));
    }

    auto &ast_ctx{ __rt_ctx->jit_prc.interpreter->getCI()->getASTContext() };
    auto const name{ ast_ctx.DeclarationNames.getCXXOperatorName(
      static_cast<clang::OverloadedOperatorKind>(expr->op)) };
    return gen_aot_call(Cpp::MakeBuiltinOperatorAotCallable(static_cast<Cpp::Operator>(expr->op),
                                                            expr->type,
                                                            arg_types,
                                                            unique_munged_string()),
                        nullptr,
                        expr->type,
                        name.getAsString(),
                        expr->arg_exprs,
                        expr->position,
                        expr->kind,
                        arity);
  }

  llvm::Value *llvm_processor::impl::gen(analyze::expr::cpp_box_ref const expr,
                                         analyze::expr::function_arity const &arity)
  {
    auto const expr_type{ cpp_util::expression_type(expr->value_expr) };
    auto value{ ctx->builder->CreateLoad(ctx->builder->getPtrTy(), gen(expr->value_expr, arity)) };

    /* We want to be sure that we're only boxing pointers, so if we have a reference, we need
     * to get past it. */
    if(Cpp::IsReferenceType(expr_type))
    {
      value = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), value);
    }
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(),
                              { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                              false));
    auto const fn(llvm_module->getOrInsertFunction("jank_box", fn_type));

    auto const type_str{ gen_c_string(
      Cpp::GetTypeAsString(Cpp::GetCanonicalType(Cpp::GetNonReferenceType(expr_type)))) };
    llvm::SmallVector<llvm::Value *, 2> const args{ type_str, value };
    auto const call(ctx->builder->CreateCall(fn, args));

    {
      auto const set_meta_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_meta_fn(llvm_module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

      /* TODO: Can strip here, when the flag is enabled: strip_source_from_meta
         * Otherwise, we need this info for unboxing errors. */
      auto const meta(gen_global_from_read_string(source_to_meta(expr->source)));
      ctx->builder->CreateCall(set_meta_fn, { call, meta });
    }

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::impl::gen(analyze::expr::cpp_unbox_ref const expr,
                                         analyze::expr::function_arity const &arity)
  {
    auto const alloc{ ctx->builder->CreateAlloca(
      ctx->builder->getPtrTy(),
      llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
    auto const value{ gen(expr->value_expr, arity) };
    auto const fn_type(llvm::FunctionType::get(
      ctx->builder->getPtrTy(),
      { ctx->builder->getPtrTy(), ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
      false));
    auto const fn(llvm_module->getOrInsertFunction("jank_unbox_with_source", fn_type));

    auto const type_str{ gen_c_string(Cpp::GetTypeAsString(Cpp::GetCanonicalType(expr->type))) };
    auto const source_meta{ gen_global_from_read_string(source_to_meta(expr->source)) };
    llvm::SmallVector<llvm::Value *, 3> const args{ type_str, value, source_meta };
    auto const call(ctx->builder->CreateCall(fn, args));
    ctx->builder->CreateStore(call, alloc);

    if(expr->position == expression_position::tail)
    {
      return gen_ret(call);
    }

    return alloc;
  }

  llvm::Value *llvm_processor::impl::gen(analyze::expr::cpp_new_ref const expr,
                                         analyze::expr::function_arity const &arity)
  {
    auto const alloc{ ctx->builder->CreateAlloca(
      ctx->builder->getPtrTy(),
      llvm::ConstantInt::get(ctx->builder->getInt64Ty(), 1)) };
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, false));
    auto const fn(llvm_module->getOrInsertFunction("GC_malloc", fn_type));

    auto const size{ Cpp::GetSizeOfType(expr->type) };
    llvm::SmallVector<llvm::Value *, 1> const args{
      llvm::ConstantInt::get(ctx->builder->getInt64Ty(), size)
    };
    auto const gc_alloc(ctx->builder->CreateCall(fn, args));
    ctx->builder->CreateStore(gc_alloc, alloc);

    if(!Cpp::IsTriviallyDestructible(expr->type))
    {
      auto const dtor{ Cpp::GetDestructor(Cpp::GetScopeFromType(expr->type)) };
      auto const dtor_callable{ Cpp::MakeAotCallable(dtor, unique_munged_string()) };
      link_module(*ctx, reinterpret_cast<llvm::Module *>(dtor_callable.getModule()));

      auto const reg_fn_type(llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                                     { ctx->builder->getPtrTy(),
                                                       ctx->builder->getPtrTy(),
                                                       ctx->builder->getPtrTy(),
                                                       ctx->builder->getPtrTy(),
                                                       ctx->builder->getPtrTy() },
                                                     false));
      auto const reg_fn(
        llvm_module->getOrInsertFunction("GC_register_finalizer_ignore_self", reg_fn_type));

      auto const finalizer_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto finalizer_fn(
        llvm_module->getOrInsertFunction(dtor_callable.getName(), finalizer_fn_type));

      auto const null{ llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()) };
      llvm::SmallVector<llvm::Value *, 5> const reg_args{
        gc_alloc,
        ctx->builder->CreateBitCast(finalizer_fn.getCallee(), ctx->builder->getPtrTy()),
        null,
        null,
        null
      };
      ctx->builder->CreateCall(reg_fn, reg_args);
    }

    auto const ctor{ llvm::cast<expr::cpp_constructor_call>(expr->value_expr.data) };
    gen(ctor, arity, gc_alloc);
    return alloc;
  }

  llvm::Value *llvm_processor::impl::gen(analyze::expr::cpp_delete_ref const expr,
                                         analyze::expr::function_arity const &arity)
  {
    auto const val_alloc{ gen(expr->value_expr, arity) };
    auto const val{ ctx->builder->CreateLoad(ctx->builder->getPtrTy(), val_alloc) };
    auto const value_type{ Cpp::GetPointeeType(cpp_util::expression_type(expr->value_expr)) };

    /* Calling GC_free won't trigger the finalizer. Not sure why, but it's explicitly
     * documented in bdwgc. So, we'll invoke it manually if needed, prior to GC_free. */
    if(!Cpp::IsTriviallyDestructible(value_type))
    {
      auto const dtor{ Cpp::GetDestructor(Cpp::GetScopeFromType(value_type)) };
      auto const dtor_callable{ Cpp::MakeAotCallable(dtor, unique_munged_string()) };
      link_module(*ctx, reinterpret_cast<llvm::Module *>(dtor_callable.getModule()));

      auto const dtor_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const dtor_fn(llvm_module->getOrInsertFunction(dtor_callable.getName(), dtor_fn_type));

      auto const null{ llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()) };
      llvm::SmallVector<llvm::Value *, 2> const dtor_args{ val, null };
      ctx->builder->CreateCall(dtor_fn, dtor_args);
    }

    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false));
    auto const fn(llvm_module->getOrInsertFunction("GC_free", fn_type));

    llvm::SmallVector<llvm::Value *, 1> const args{ val };
    ctx->builder->CreateCall(fn, args);

    auto const ret{ gen_global(jank_nil()) };
    if(expr->position == expression_position::tail)
    {
      return gen_ret(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::impl::gen_var(obj::symbol_ref const qualified_name) const
  {
    auto const found(ctx->var_globals.find(qualified_name));
    if(found != ctx->var_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->var_globals[qualified_name]);
    auto const name(util::format("var_{}", munge(qualified_name->to_string())));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const fn(llvm_module->getOrInsertFunction("jank_var_intern_c", fn_type));

      llvm::SmallVector<llvm::Value *, 2> const args{ gen_c_string(qualified_name->ns),
                                                      gen_c_string(qualified_name->name) };
      auto const call(ctx->builder->CreateCall(fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_var_root(obj::symbol_ref const qualified_name,
                                                  var_root_kind const kind) const
  {
    auto it(ctx->var_root_globals.find(qualified_name));
    if(it != ctx->var_root_globals.end())
    {
      /* In the global ctor, we want to return a load of the var-root if the global var-root
       * already exists. In other contexts, the load is unnecessary so we can
       * just return the var-root value directly.
       */
      if(kind == var_root_kind::global_init)
      {
        return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), it->second);
      }
      return it->second;
    }
    auto const name(util::format("var_root_{}", munge(qualified_name->to_string())));
    auto const var_root(create_global_var(name));
    llvm_module->insertGlobalVariable(var_root);
    auto &global(ctx->var_root_globals[qualified_name]);
    global = var_root;

    /* When generating a var-root in a context where we are already calling var_bind_root,
     * we can just return the global var-root. */
    if(kind == var_root_kind::binded_def)
    {
      return global;
    }

    /* When generating a var-root in the "jank_load" function, we just need to directly deref
     * the global var and return the global var-root. */
    if(kind == var_root_kind::load_init)
    {
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const fn(llvm_module->getOrInsertFunction("jank_deref", fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_var(qualified_name) };
      auto const call(ctx->builder->CreateCall(fn, args));
      ctx->builder->CreateStore(call, global);
      return global;
    }

    /* Similar to generating a var, we set the var-root in the global ctor block by derefing
     * the global var and return a load to the var-root unless the previous
     * block was the global ctor. */
    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const fn(llvm_module->getOrInsertFunction("jank_deref", fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_var(qualified_name) };
      auto const call(ctx->builder->CreateCall(fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }
    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_c_string(jtl::immutable_string const &s) const
  {
    auto const found(ctx->c_string_globals.find(s));
    if(found != ctx->c_string_globals.end())
    {
      return found->second;
    }
    return ctx->c_string_globals[s] = ctx->builder->CreateGlobalString(s.c_str());
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::nil_ref const nil) const
  {
    auto const found(ctx->literal_globals.find(nil));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[nil]);
    auto const name("nil");
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_const_nil", create_fn_type));
      auto const call(ctx->builder->CreateCall(create_fn));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::boolean_ref const b) const
  {
    auto const found(ctx->literal_globals.find(b));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[b]);
    auto const name(b->data ? "true" : "false");
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), false));
      /* We turn the literal value into jank_const_true / jank_const_false. */
      auto const fn_name{ util::format("jank_const_{}", name) };
      auto const create_fn(llvm_module->getOrInsertFunction(fn_name.c_str(), create_fn_type));
      auto const call(ctx->builder->CreateCall(create_fn));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::integer_ref const i) const
  {
    auto const found(ctx->literal_globals.find(i));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[i]);
    auto const name(util::format("int_{}", i->data));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_integer_create", create_fn_type));
      auto const arg(llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), i->data));
      auto const call(ctx->builder->CreateCall(create_fn, { arg }));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::big_integer_ref const i) const
  {
    auto const found(ctx->literal_globals.find(i));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[i]);
    auto const name(util::format("big_integer_{}", i->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(
        llvm_module->getOrInsertFunction("jank_big_integer_create", create_fn_type));

      auto const str_repr(i->to_string());
      auto const c_str_arg(gen_c_string(str_repr));

      llvm::SmallVector<llvm::Value *, 1> const args{ c_str_arg };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::big_decimal_ref const i) const
  {
    auto const found(ctx->literal_globals.find(i));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[i]);
    auto const name(util::format("big_decimal_{}", i->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(
        llvm_module->getOrInsertFunction("jank_big_decimal_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(i->to_string().c_str()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::real_ref const r) const
  {
    auto const found(ctx->literal_globals.find(r));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[r]);
    auto const name(util::format("real_{}", r->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getDoubleTy() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_real_create", create_fn_type));
      auto const arg(llvm::ConstantFP::get(ctx->builder->getDoubleTy(), r->data));
      auto const call(ctx->builder->CreateCall(create_fn, { arg }));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::ratio_ref const r) const
  {
    if(auto const found(ctx->literal_globals.find(r)); found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[r]);
    auto const name(util::format("ratio_{}", r->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_ratio_create", create_fn_type));
      llvm::SmallVector<llvm::Value *, 2> const args{ gen_global(make_box(r->data.numerator)),
                                                      gen_global(make_box(r->data.denominator)) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::persistent_string_ref const s) const
  {
    auto const found(ctx->literal_globals.find(s));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[s]);
    auto const name(util::format("string_{}", s->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_string_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(s->data.c_str()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::re_pattern_ref const re) const
  {
    auto const found(ctx->literal_globals.find(re));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[re]);
    auto const name(util::format("re_pattern_{}", re->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_regex_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(re->to_string().c_str()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::uuid_ref const u) const
  {
    auto const found(ctx->literal_globals.find(u));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[u]);
    auto const name(util::format("uuid_{}", u->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_uuid_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(u->to_string().c_str()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::inst_ref const i) const
  {
    auto const found(ctx->literal_globals.find(i));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[i]);
    auto const name(util::format("inst_{}", i->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_inst_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(i->to_string().c_str()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::symbol_ref const s) const
  {
    auto const found(ctx->literal_globals.find(s));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[s]);
    auto const name(util::format("symbol_{}", s->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_symbol_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 2> const args{ gen_global(make_box(s->ns)),
                                                      gen_global(make_box(s->name)) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(s->meta)
      {
        auto const set_meta_fn_type(
          llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                  { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                  false));
        auto const set_meta_fn(llvm_module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

        /* TODO: Can strip here, when the flag is enabled: strip_source_from_meta
         * Otherwise, we need this info for macro expansion errors. i.e. `(foo ~'bar) */
        auto const meta(gen_global_from_read_string(s->meta.unwrap()));
        ctx->builder->CreateCall(set_meta_fn, { call, meta });
      }

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::keyword_ref const k) const
  {
    auto const found(ctx->literal_globals.find(k));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[k]);
    auto const name(util::format("keyword_{}", k->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_keyword_intern", create_fn_type));

      llvm::SmallVector<llvm::Value *, 2> const args{ gen_global(make_box(k->sym->ns)),
                                                      gen_global(make_box(k->sym->name)) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global(obj::character_ref const c) const
  {
    auto const found(ctx->literal_globals.find(c));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[c]);
    auto const name(util::format("char_{}", c->to_hash()));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(
        llvm_module->getOrInsertFunction("jank_character_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(c->to_code_string()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_global_from_read_string(object_ref const o) const
  {
    auto const found(ctx->literal_globals.find(o));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[o]);
    auto const name(util::format("data_{}", to_hash(o)));
    auto const var(create_global_var(name));
    llvm_module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_read_string_c", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(runtime::to_code_string(o)) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      runtime::visit_object(
        [&](auto const typed_o) {
          using T = decltype(typed_o)::value_type;

          if constexpr(behavior::metadatable<T>)
          {
            if(typed_o->meta)
            {
              auto const set_meta_fn_type(
                llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                        { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                        false));
              auto const set_meta_fn(
                llvm_module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

              /* TODO: This shouldn't be its own global; we don't need to reference it later. */
              auto const meta(
                gen_global_from_read_string(strip_source_from_meta(typed_o->meta.unwrap())));
              auto const meta_name(util::format("{}_meta", name));
              meta->setName(meta_name.c_str());
              ctx->builder->CreateCall(set_meta_fn, { call, meta });
            }
          }
        },
        o);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::impl::gen_function_instance(expr::function_ref const expr,
                                                           expr::function_arity const &fn_arity)
  {
    expr::function_arity const *variadic_arity{};
    expr::function_arity const *highest_fixed_arity{};
    auto const captures(expr->captures());
    jank_debug_assert(!expr->arities.empty());
    for(auto const &arity : expr->arities)
    {
      if(arity.fn_ctx->is_variadic)
      {
        variadic_arity = &arity;
      }
      else if(!highest_fixed_arity
              || highest_fixed_arity->fn_ctx->param_count < arity.fn_ctx->param_count)
      {
        highest_fixed_arity = &arity;
      }
    }
    auto const variadic_ambiguous(highest_fixed_arity && variadic_arity
                                  && highest_fixed_arity->fn_ctx->param_count
                                    == variadic_arity->fn_ctx->param_count - 1);

    /* If there's a variadic arity, the highest fixed args is however many precede the "rest"
     * args. Otherwise, the highest fixed args is just the highest fixed arity. */
    auto const highest_fixed_args(variadic_arity
                                    ? variadic_arity->fn_ctx->param_count - 1
                                    /* This is only a concern if expr->arities is empty,
                                     * which will never happen.
                                     * NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage) */
                                    : highest_fixed_arity->fn_ctx->param_count);

    auto const arity_flags_fn_type(llvm::FunctionType::get(
      ctx->builder->getInt8Ty(),
      { ctx->builder->getInt8Ty(), ctx->builder->getInt8Ty(), ctx->builder->getInt8Ty() },
      false));
    auto const arity_flags_fn(
      llvm_module->getOrInsertFunction("jank_function_build_arity_flags", arity_flags_fn_type));
    auto const arity_flags(ctx->builder->CreateCall(arity_flags_fn,
                                                    { ctx->builder->getInt8(highest_fixed_args),
                                                      ctx->builder->getInt8(!!variadic_arity),
                                                      ctx->builder->getInt8(variadic_ambiguous) }));

    llvm::Value *fn_obj{};

    auto const is_closure(!captures.empty());

    if(!is_closure)
    {
      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt8Ty() }, false));
      auto const create_fn(
        llvm_module->getOrInsertFunction("jank_function_create", create_fn_type));
      fn_obj = ctx->builder->CreateCall(create_fn, { arity_flags });
    }
    else
    {
      std::vector<llvm::Type *> const capture_types{ captures.size(), ctx->builder->getPtrTy() };
      auto const closure_ctx_type(
        get_or_insert_struct_type(util::format("{}_context", munge(expr->unique_name)),
                                  capture_types));

      auto const malloc_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, false));
      auto const malloc_fn(llvm_module->getOrInsertFunction("GC_malloc", malloc_fn_type));
      auto const closure_obj(
        ctx->builder->CreateCall(malloc_fn, { llvm::ConstantExpr::getSizeOf(closure_ctx_type) }));

      usize index{};
      for(auto const &capture : captures)
      {
        auto const field_ptr(ctx->builder->CreateStructGEP(closure_ctx_type, closure_obj, index++));
        auto const name(capture.first);
        if(!locals.contains(name))
        {
          deferred_inits.emplace_back(expr, name, capture.second, field_ptr);
        }
        else
        {
          expr::local_reference const local_ref{ expression_position::value,
                                                 expr->frame,
                                                 true,
                                                 name,
                                                 capture.second };
          auto local{ gen(expr::local_reference_ref{ &local_ref }, fn_arity) };
          if(llvm::isa<llvm::AllocaInst>(local) && cpp_util::is_any_object(capture.second->type))
          {
            local = ctx->builder->CreateLoad(ctx->builder->getPtrTy(), local);
          }

          ctx->builder->CreateStore(local, field_ptr);
        }
      }

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getInt8Ty(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(llvm_module->getOrInsertFunction("jank_closure_create", create_fn_type));
      fn_obj = ctx->builder->CreateCall(create_fn, { arity_flags, closure_obj });
    }

    for(auto const &arity : expr->arities)
    {
      auto const set_arity_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_arity_fn_name{
        is_closure ? util::format("jank_closure_set_arity{}", arity.params.size())
                   : util::format("jank_function_set_arity{}", arity.params.size())
      };
      auto const set_arity_fn(
        llvm_module->getOrInsertFunction(set_arity_fn_name.c_str(), set_arity_fn_type));

      /* We add one for `this`. */
      std::vector<llvm::Type *> const target_arg_types{ arity.params.size() + 1,
                                                        ctx->builder->getPtrTy() };
      auto const target_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), target_arg_types, false));
      auto const target_fn_name{
        util::format("{}_{}", munge(expr->unique_name), arity.params.size())
      };
      auto target_fn(llvm_module->getOrInsertFunction(target_fn_name.c_str(), target_fn_type));

      ctx->builder->CreateCall(set_arity_fn, { fn_obj, target_fn.getCallee() });
    }

    if(expr->meta.is_some())
    {
      auto const set_meta_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_meta_fn(llvm_module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

      auto const meta(gen_global_from_read_string(strip_source_from_meta(expr->meta)));
      ctx->builder->CreateCall(set_meta_fn, { fn_obj, meta });
    }

    return fn_obj;
  }

  void llvm_processor::impl::create_global_ctor() const
  {
    auto const init_type(llvm::FunctionType::get(ctx->builder->getVoidTy(), false));
    auto const init(llvm::Function::Create(init_type,
                                           llvm::Function::ExternalLinkage,
                                           ctx->ctor_name.c_str(),
                                           *llvm_module));
    ctx->global_ctor_block->insertInto(init);

    /* XXX: Modules are written to object files, which can't use global ctors until
     * we're on the ORC runtime. Instead, we just generate our load function to call
     * our global ctor first. */
    if(target != compilation_target::module)
    {
      llvm::appendToGlobalCtors(*llvm_module, init, 65535);
    }

    if(profile::is_enabled())
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false));
      auto const fn(llvm_module->getOrInsertFunction("jank_profile_enter", fn_type));
      ctx->builder->CreateCall(fn,
                               { gen_c_string(util::format("global ctor for {}", root_fn->name)) });
    }
  }

  llvm::GlobalVariable *
  llvm_processor::impl::create_global_var(jtl::immutable_string const &name) const
  {
    return new llvm::GlobalVariable{ ctx->builder->getPtrTy(),
                                     false,
                                     llvm::GlobalVariable::InternalLinkage,
                                     llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()),
                                     name.c_str() };
  }

  llvm::StructType *
  llvm_processor::impl::get_or_insert_struct_type(std::string const &name,
                                                  std::vector<llvm::Type *> const &fields) const
  {
    auto const found(llvm::StructType::getTypeByName(*llvm_ctx, name));
    if(found)
    {
      return found;
    }

    std::vector<llvm::Type *> const field_types{ fields.size(), ctx->builder->getPtrTy() };
    auto const struct_type(llvm::StructType::create(field_types, name));
    return struct_type;
  }

  util::scope_exit llvm_processor::impl::gen_stack_save()
  {
    /* In some cases, such as loops, we use LLVM's stack preservation intrinsics.
     * These will help us reset the stack on each iteration so that each new alloc
     * doesn't actually keep grabbing more stack space.
     *
     * However, there is some tricky logic in balancing each save/restore, since we
     * can have multiple terminators in an IR function and we need to make sure that
     * each of them gets a restore. Also, we can have nested saves and we need to
     * make sure they get restored in the correct order. */
    auto const stack_ptr{ ctx->builder->CreateStackSave() };
    stack_saves.emplace_back(stack_ptr);

    /* The main logic here is to pop the back of the stack, but we add some error handling
     * as well, to detect cases where we're not restoring in the correct order. */
    return { [stack_ptr, this]() {
      ssize found{ -1 };
      for(ssize i{}; std::cmp_not_equal(i, stack_saves.size()); ++i)
      {
        if(stack_saves[i].data == stack_ptr)
        {
          found = i;
        }
      }

      jank_debug_assert(found == -1 || found == static_cast<ssize>(stack_saves.size()) - 1);
      if(found != -1)
      {
        stack_saves.erase(stack_saves.begin() + found);
      }
    } };
  }

  void llvm_processor::impl::gen_stack_restore()
  {
    jank_debug_assert(!stack_saves.empty());
    ctx->builder->CreateStackRestore(stack_saves.back());
  }

  jtl::ptr<llvm::Value> llvm_processor::impl::gen_ret(jtl::ptr<llvm::Value> const value)
  {
    for(auto it{ stack_saves.rbegin() }; it != stack_saves.rend(); ++it)
    {
      ctx->builder->CreateStackRestore(*it);
    }
    return ctx->builder->CreateRet(value);
  }

  jtl::ptr<llvm::Value> llvm_processor::impl::gen_ret()
  {
    for(auto it{ stack_saves.rbegin() }; it != stack_saves.rend(); ++it)
    {
      ctx->builder->CreateStackRestore(*it);
    }
    return ctx->builder->CreateRetVoid();
  }

  void llvm_processor::optimize() const
  {
    jtl::immutable_string_view const print_settings{ getenv("JANK_PRINT_IR") ?: "" };
    if(print_settings == "1")
    {
      print();
    }

#ifdef JANK_ASSERTIONS_ENABLED
    if(llvm::verifyModule(*_impl->llvm_module, &llvm::errs()))
    {
      std::cerr << "----------\n";
      print();
      std::cerr << "----------\n";
    }
#endif

    _impl->ctx->mpm.run(*_impl->llvm_module, *_impl->ctx->mam);

    if(print_settings == "2")
    {
      print();
    }
  }

  void llvm_processor::print() const
  {
    _impl->llvm_module->print(llvm::outs(), nullptr);
  }

  llvm::orc::ThreadSafeModule &llvm_processor::get_module() const
  {
    return _impl->ctx->module;
  }

  jtl::immutable_string const &llvm_processor::get_module_name() const
  {
    return _impl->ctx->module_name;
  }

  jtl::immutable_string llvm_processor::get_root_fn_name() const
  {
    return _impl->root_fn->unique_name;
  }
}
