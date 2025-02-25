#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/evaluate.hpp>
#include <jank/profile/time.hpp>

/* TODO: Remove exceptions. */
namespace jank::codegen
{
  using namespace jank::analyze;

  reusable_context::reusable_context(native_persistent_string const &module_name)
    : module_name{ module_name }
    , ctor_name{ runtime::munge(__rt_ctx->unique_string("jank_global_init")) }
    , llvm_ctx{ std::make_unique<llvm::LLVMContext>() }
    , module{ std::make_unique<llvm::Module>(__rt_ctx->unique_string(module_name).c_str(),
                                             *llvm_ctx) }
    , builder{ std::make_unique<llvm::IRBuilder<>>(*llvm_ctx) }
    , global_ctor_block{ llvm::BasicBlock::Create(*llvm_ctx, "entry") }
    , fpm{ std::make_unique<llvm::FunctionPassManager>() }
    , lam{ std::make_unique<llvm::LoopAnalysisManager>() }
    , fam{ std::make_unique<llvm::FunctionAnalysisManager>() }
    , cgam{ std::make_unique<llvm::CGSCCAnalysisManager>() }
    , mam{ std::make_unique<llvm::ModuleAnalysisManager>() }
    , pic{ std::make_unique<llvm::PassInstrumentationCallbacks>() }
    , si{ std::make_unique<llvm::StandardInstrumentations>(*llvm_ctx,
                                                           /*DebugLogging*/ true) }
  {
    /* The LLVM front-end tips documentation suggests setting the target triple and
     * data layout to improve back-end codegen performance. */
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    module->setDataLayout(
      __rt_ctx->jit_prc.interpreter->getExecutionEngine().get().getDataLayout());

    /* TODO: Configure these passes based on the CLI optimization flag. */

    /* TODO: Add more passes and measure the order of the passes. */

    si->registerCallbacks(*pic, mam.get());

    /* Do simple "peephole" optimizations and bit-twiddling optzns. */
    fpm->addPass(llvm::InstCombinePass());
    /* Reassociate expressions. */
    fpm->addPass(llvm::ReassociatePass());
    /* Eliminate Common SubExpressions. */
    fpm->addPass(llvm::GVNPass());
    /* Simplify the control flow graph (deleting unreachable blocks, etc). */
    fpm->addPass(llvm::SimplifyCFGPass());

    llvm::PassBuilder pb;
    pb.registerModuleAnalyses(*mam);
    pb.registerFunctionAnalyses(*fam);
    pb.crossRegisterProxies(*lam, *fam, *cgam, *mam);
  }

  llvm_processor::llvm_processor(expression_ptr const &expr,
                                 native_persistent_string const &module_name,
                                 compilation_target const target)
    : llvm_processor{ boost::get<expr::function<expression>>(expr->data), module_name, target }
  {
  }

  llvm_processor::llvm_processor(expr::function<expression> const &expr,
                                 native_persistent_string const &module_name,
                                 compilation_target const target)
    : target{ target }
    , root_fn{ expr }
    , ctx{ std::make_unique<reusable_context>(module_name) }
  {
    assert(root_fn.frame.data);
  }

  llvm_processor::llvm_processor(expr::function<expression> const &expr,
                                 std::unique_ptr<reusable_context> ctx)
    : target{ compilation_target::function }
    , root_fn{ expr }
    , ctx{ std::move(ctx) }
  {
  }

  void llvm_processor::create_function()
  {
    auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), false));
    auto const name(munge(root_fn.unique_name));
    fn = llvm::Function::Create(fn_type,
                                llvm::Function::ExternalLinkage,
                                name.c_str(),
                                *ctx->module);

    auto const entry(llvm::BasicBlock::Create(*ctx->llvm_ctx, "entry", fn));
    ctx->builder->SetInsertPoint(entry);
  }

  void llvm_processor::create_function(expr::function_arity<expression> const &arity)
  {
    auto const captures(root_fn.captures());
    auto const is_closure(!captures.empty());

    /* Closures get one extra parameter, the first one, which is a pointer to the closure's
     * context. The context is a struct containing all captured values. */
    std::vector<llvm::Type *> const arg_types{ arity.params.size() + is_closure,
                                               ctx->builder->getPtrTy() };
    auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
    std::string const name{ munge(root_fn.unique_name) };
    auto fn_value(ctx->module->getOrInsertFunction(
      target == compilation_target::module ? name : fmt::format("{}_{}", name, arity.params.size()),
      fn_type));
    fn = llvm::cast<llvm::Function>(fn_value.getCallee());
    fn->setLinkage(llvm::Function::ExternalLinkage);

    auto const entry(llvm::BasicBlock::Create(*ctx->llvm_ctx, "entry", fn));
    ctx->builder->SetInsertPoint(entry);

    /* JIT loaded object files don't support global ctors, so we need to call our manually.
     * Fortunately, we have our load function which we can hook into. So, if we're compiling
     * a module and we've just created the load function fo that module, the first thing
     * we want to do is call our global ctor. */
    if(target == compilation_target::module
       && root_fn.unique_name == module::module_to_load_function(ctx->module_name))
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
       * on intial load.
       */
      auto const current_ns{ __rt_ctx->current_ns() };
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getInt64Ty() },
                                false));
      auto const fn(ctx->module->getOrInsertFunction("jank_ns_set_symbol_counter", fn_type));

      ctx->builder->CreateCall(
        fn,
        { gen_c_string(current_ns->name->get_name()),
          llvm::ConstantInt::get(ctx->builder->getInt64Ty(), current_ns->symbol_counter.load()) });
    }

    for(size_t i{}; i < arity.params.size(); ++i)
    {
      auto &param(arity.params[i]);
      auto arg(fn->getArg(i + is_closure));
      arg->setName(param->get_name().c_str());
      locals[param] = arg;
    }

    if(is_closure)
    {
      auto const context(fn->getArg(0));
      auto const captures(root_fn.captures());
      std::vector<llvm::Type *> const capture_types{ captures.size(), ctx->builder->getPtrTy() };
      auto const closure_ctx_type(
        get_or_insert_struct_type(fmt::format("{}_context", munge(root_fn.unique_name)),
                                  capture_types));
      size_t index{};
      for(auto const &capture : captures)
      {
        auto const field_ptr(ctx->builder->CreateStructGEP(closure_ctx_type, context, index++));
        locals[capture.first] = ctx->builder->CreateLoad(ctx->builder->getPtrTy(),
                                                         field_ptr,
                                                         capture.first->name.c_str());
      }
    }
  }

  string_result<void> llvm_processor::gen()
  {
    profile::timer const timer{ "ir gen" };
    if(target != compilation_target::function)
    {
      create_global_ctor();
    }

    for(auto const &arity : root_fn.arities)
    {
      /* TODO: Add profiling to the fn body? Need to exit on every return. */
      create_function(arity);
      for(auto const &form : arity.body.values)
      {
        gen(form, arity);
      }

      /* If we have an empty function, ensure we're still returning nil. */
      if(arity.body.values.empty())
      {
        ctx->builder->CreateRet(gen_global(obj::nil::nil_const()));
      }
    }

    if(target == compilation_target::eval)
    {
      //to_string();
    }

    /* Run our optimization passes on the function, mutating it. */
    ctx->fpm->run(*fn, *ctx->fam);

    if(target != compilation_target::function)
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      if(profile::is_enabled())
      {
        auto const fn_type(
          llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false));
        auto const fn(ctx->module->getOrInsertFunction("jank_profile_exit", fn_type));
        ctx->builder->CreateCall(fn,
                                 { gen_c_string(fmt::format("global ctor for {}", root_fn.name)) });
      }

      ctx->builder->CreateRetVoid();

      if(llvm::verifyModule(*ctx->module, &llvm::errs()))
      {
        std::cerr << "----------\n";
        to_string();
        std::cerr << "----------\n";
        return err(fmt::format("invalid IR module {}", ctx->module_name));
      }
    }

    return ok();
  }

  llvm::Value *
  llvm_processor::gen(expression_ptr const &ex, expr::function_arity<expression> const &fn_arity)
  {
    llvm::Value *ret{};
    boost::apply_visitor(
      [this, fn_arity, &ret](auto const &typed_ex) { ret = gen(typed_ex, fn_arity); },
      ex->data);
    return ret;
  }

  llvm::Value *llvm_processor::gen(expr::def<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const ref(gen_var(expr.name));

    if(expr.value.is_some())
    {
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const fn(ctx->module->getOrInsertFunction("jank_var_bind_root", fn_type));

      llvm::SmallVector<llvm::Value *, 2> const args{ ref, gen(expr.value.unwrap(), arity) };
      ctx->builder->CreateCall(fn, args);
    }

    option<std::reference_wrapper<lifted_constant const>> meta;
    if(expr.name->meta.is_some())
    {
      meta = expr.frame->find_lifted_constant(expr.name->meta.unwrap()).unwrap();

      auto const set_meta_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_meta_fn(ctx->module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

      auto const meta(gen_global_from_read_string(expr.name->meta.unwrap()));
      ctx->builder->CreateCall(set_meta_fn, { ref, meta });
    }

    auto const set_dynamic_fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(),
                              { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                              false));

    auto const set_dynamic_fn(
      ctx->module->getOrInsertFunction("jank_var_set_dynamic", set_dynamic_fn_type));

    auto const dynamic{ truthy(get(expr.name->meta.unwrap_or(obj::nil::nil_const()),
                                   __rt_ctx->intern_keyword("dynamic").expect_ok())) };

    auto const dynamic_global{ gen_global(make_box(dynamic)) };

    ctx->builder->CreateCall(set_dynamic_fn, { ref, dynamic_global });

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(ref);
    }

    return ref;
  }

  llvm::Value *llvm_processor::gen(expr::var_deref<expression> const &expr,
                                   expr::function_arity<expression> const &) const
  {
    auto const ref(gen_var(make_box<obj::symbol>(expr.var->n, expr.var->name)));
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
    auto const fn(ctx->module->getOrInsertFunction("jank_deref", fn_type));

    llvm::SmallVector<llvm::Value *, 1> const args{ ref };
    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::var_ref<expression> const &expr,
                                   expr::function_arity<expression> const &) const
  {
    auto const var(gen_var(expr.qualified_name));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(var);
    }

    return var;
  }

  static native_persistent_string arity_to_call_fn(size_t const arity)
  {
    /* Anything max_params + 1 or higher gets packed into a list so we
     * just end up calling max_params + 1 at most. */
    switch(arity)
    {
      case 0 ... runtime::max_params:
        return fmt::format("jank_call{}", arity);
      default:
        return fmt::format("jank_call{}", runtime::max_params + 1);
    }
  }

  llvm::Value *llvm_processor::gen(expr::call<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const callee(gen(expr.source_expr, arity));

    llvm::SmallVector<llvm::Value *> arg_handles;
    llvm::SmallVector<llvm::Type *> arg_types;
    arg_handles.reserve(expr.arg_exprs.size() + 1);
    arg_types.reserve(expr.arg_exprs.size() + 1);

    arg_handles.emplace_back(callee);
    arg_types.emplace_back(ctx->builder->getPtrTy());

    for(auto const &arg_expr : expr.arg_exprs)
    {
      arg_handles.emplace_back(gen(arg_expr, arity));
      arg_types.emplace_back(ctx->builder->getPtrTy());
    }

    auto const call_fn_name(arity_to_call_fn(expr.arg_exprs.size()));

    auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
    auto const fn(ctx->module->getOrInsertFunction(call_fn_name.c_str(), fn_type));
    auto const call(ctx->builder->CreateCall(fn, arg_handles));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::primitive_literal<expression> const &expr,
                                   expr::function_arity<expression> const &)
  {
    auto const ret(runtime::visit_object(
      [&](auto const typed_o) -> llvm::Value * {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, runtime::obj::nil> || std::same_as<T, runtime::obj::boolean>
                     || std::same_as<T, runtime::obj::integer>
                     || std::same_as<T, runtime::obj::real> || std::same_as<T, runtime::obj::symbol>
                     || std::same_as<T, runtime::obj::character>
                     || std::same_as<T, runtime::obj::keyword>
                     || std::same_as<T, runtime::obj::persistent_string>
                     || std::same_as<T, runtime::obj::ratio>)
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
          throw std::runtime_error{ fmt::format("unimplemented constant codegen: {}\n",
                                                typed_o->to_string()) };
        }
      },
      expr.data));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(expr::list<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(ctx->module->getOrInsertFunction("jank_list_create", fn_type));

    auto const size(expr.data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + size);
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &expr : expr.data_exprs)
    {
      args.emplace_back(gen(expr, arity));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::vector<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(ctx->module->getOrInsertFunction("jank_vector_create", fn_type));

    auto const size(expr.data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + size);
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &expr : expr.data_exprs)
    {
      args.emplace_back(gen(expr, arity));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::map<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(ctx->module->getOrInsertFunction("jank_map_create", fn_type));

    auto const size(expr.data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + (size * 2));
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &pair : expr.data_exprs)
    {
      args.emplace_back(gen(pair.first, arity));
      args.emplace_back(gen(pair.second, arity));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::set<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, true));
    auto const fn(ctx->module->getOrInsertFunction("jank_set_create", fn_type));

    auto const size(expr.data_exprs.size());
    std::vector<llvm::Value *> args;
    args.reserve(1 + size);
    args.emplace_back(ctx->builder->getInt64(size));

    for(auto const &expr : expr.data_exprs)
    {
      args.emplace_back(gen(expr, arity));
    }

    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::gen(expr::local_reference const &expr, expr::function_arity<expression> const &)
  {
    auto const ret(locals[expr.binding.name]);
    assert(ret);

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(expr::function<expression> const &expr,
                                   expr::function_arity<expression> const &fn_arity)
  {
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };

      llvm_processor nested{ expr, std::move(ctx) };
      auto const res{ nested.gen() };
      if(res.is_err())
      {
        /* TODO: Return error. */
        res.expect_ok();
      }

      ctx = std::move(nested.ctx);
    }

    auto const fn_obj(gen_function_instance(expr, fn_arity));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(fn_obj);
    }

    return fn_obj;
  }

  llvm::Value *llvm_processor::gen(expr::recur<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    /* The codegen for the special recur form is very similar to the named recursion
     * codegen, but it's simpler. The key difference is that named recursion requires
     * arg packing, whereas the special recur form does not. This means, for variadic
     * functions, the special recur form will be expected to supply a sequence for the
     * variadic argument. */
    auto const fn_expr(boost::get<expr::function<expression>>(root_fn.arities[0].fn_ctx->fn->data));
    auto const is_closure(!fn_expr.captures().empty());

    llvm::SmallVector<llvm::Value *> arg_handles;
    llvm::SmallVector<llvm::Type *> arg_types;
    arg_handles.reserve(expr.arg_exprs.size() + is_closure);
    arg_types.reserve(expr.arg_exprs.size() + is_closure);

    if(is_closure)
    {
      arg_handles.emplace_back(ctx->builder->GetInsertBlock()->getParent()->getArg(0));
      arg_types.emplace_back(ctx->builder->getPtrTy());
    }

    for(auto const &arg_expr : expr.arg_exprs)
    {
      arg_handles.emplace_back(gen(arg_expr, arity));
      arg_types.emplace_back(ctx->builder->getPtrTy());
    }

    auto const call_fn_name(
      fmt::format("{}_{}", munge(fn_expr.unique_name), expr.arg_exprs.size()));
    auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
    auto const fn(ctx->module->getOrInsertFunction(call_fn_name, fn_type));
    auto const call(ctx->builder->CreateCall(fn, arg_handles));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::recursion_reference<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    /* With each recursion reference, we generate a new function instance. This is different
     * from what Clojure does, but is functionally the same so long as one doesn't rely on
     * identity checks for this sort of thing.
     *
     * We generate a new fn instance because the C fns generated for a jank fn don't belong
     * inside of a class which has a `this` which can just be used. They're standalone. So,
     * if you want an instance of that fn within the fn itself, we need to make one. For
     * closures, this will copy the current context to the new one. */
    auto const fn_obj(
      gen_function_instance(boost::get<expr::function<expression>>(expr.fn_ctx->fn->data), arity));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(fn_obj);
    }

    return fn_obj;
  }

  llvm::Value *llvm_processor::gen(expr::named_recursion<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const fn_expr(boost::get<expr::function<expression>>(expr.recursion_ref.fn_ctx->fn->data));
    auto const &captures(fn_expr.captures());

    /* Named recursion is a special kind of call. We can't go always through a var, since there
     * may not be one. We can't just use the fn's name, since we could be recursing into a
     * different arity. Finally, we need to keep in account whether or not this fn is a closure.
     *
     * For named recursion calls, we don't use dynamic_call. We just call the generated C fn
     * directly. This doesn't impede interactivity, since the whole thing will be redefined
     * if a new fn is created. */
    auto const is_closure(!captures.empty());

    /* We may have a named recursion in a closure which crosses another function in order to
     * recurse. For example:
     *
     * ```clojure
     * (let [a 1]
     *   (fn foo []
     *     (fn bar []
     *       (println a)
     *       (foo))))
     * ```
     *
     * Here, the `(foo)` call is a named recursion, but we're not actually in the `foo` fn.
     * We need to "cross" `bar` in order to get back into `foo`. This is an important
     * distinction, since the closure context for `foo` and `bar` may be different, such
     * as if `bar` closes over more data than `foo` does.
     *
     * In this case of a named recursion which crosses a fn, we can't use the current fn's
     * closure context. We need to build a new one. */
    auto const crosses_fn(
      boost::get<expr::function<expression>>(&expr.recursion_ref.fn_ctx->fn->data)
      != boost::get<expr::function<expression>>(&arity.fn_ctx->fn->data));

    llvm::SmallVector<llvm::Value *> arg_handles;
    llvm::SmallVector<llvm::Type *> arg_types;
    arg_handles.reserve(expr.arg_exprs.size() + is_closure);
    arg_types.reserve(expr.arg_exprs.size() + is_closure);

    if(arity.fn_ctx->is_variadic)
    {
      arg_handles.emplace_back(
        gen_function_instance(boost::get<expr::function<expression>>(arity.fn_ctx->fn->data),
                              arity));
      arg_types.emplace_back(ctx->builder->getPtrTy());
    }
    else if(is_closure)
    {
      /* TODO: If nested closures all build their contexts on their parents, we can always
       * pass a nested closure upward for a named recursion. This would require sorted captures
       * based on lexical scope though, which is a big jump from what we currently have. */
      if(crosses_fn)
      {
        auto const &fn(boost::get<expr::function<expression>>(expr.recursion_ref.fn_ctx->fn->data));
        std::vector<llvm::Type *> const capture_types{ captures.size(), ctx->builder->getPtrTy() };
        auto const closure_ctx_type(
          get_or_insert_struct_type(fmt::format("{}_context", munge(fn.unique_name)),
                                    capture_types));

        auto const malloc_fn_type(
          llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, false));
        auto const malloc_fn(ctx->module->getOrInsertFunction("GC_malloc", malloc_fn_type));
        auto const closure_obj(
          ctx->builder->CreateCall(malloc_fn, { llvm::ConstantExpr::getSizeOf(closure_ctx_type) }));

        size_t index{};
        for(auto const &capture : captures)
        {
          auto const field_ptr(
            ctx->builder->CreateStructGEP(closure_ctx_type, closure_obj, index++));
          expr::local_reference const local_ref{
            expression_base{ {}, expression_position::value, fn.frame },
            capture.first,
            *capture.second
          };
          ctx->builder->CreateStore(gen(local_ref, arity), field_ptr);
        }
        arg_handles.emplace_back(closure_obj);
        arg_types.emplace_back(ctx->builder->getPtrTy());
      }
      else
      {
        arg_handles.emplace_back(ctx->builder->GetInsertBlock()->getParent()->getArg(0));
        arg_types.emplace_back(ctx->builder->getPtrTy());
      }
    }

    for(auto const &arg_expr : expr.arg_exprs)
    {
      arg_handles.emplace_back(gen(arg_expr, arity));
      arg_types.emplace_back(ctx->builder->getPtrTy());
    }

    llvm::Value *call{};
    if(arity.fn_ctx->is_variadic)
    {
      auto const call_fn_name(arity_to_call_fn(expr.arg_exprs.size()));
      auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
      auto const fn(ctx->module->getOrInsertFunction(call_fn_name.c_str(), fn_type));
      call = ctx->builder->CreateCall(fn, arg_handles);
    }
    else
    {
      auto const call_fn_name(
        fmt::format("{}_{}", munge(fn_expr.unique_name), expr.arg_exprs.size()));
      auto const fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), arg_types, false));
      auto const fn(ctx->module->getOrInsertFunction(call_fn_name, fn_type));
      call = ctx->builder->CreateCall(fn, arg_handles);
    }

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(expr::let<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto old_locals(locals);
    for(auto const &pair : expr.pairs)
    {
      auto const local(expr.frame->find_local_or_capture(pair.first));
      if(local.is_none())
      {
        throw std::runtime_error{ fmt::format("ICE: unable to find local: {}",
                                              pair.first->to_string()) };
      }

      locals[pair.first] = gen(pair.second, arity);
      locals[pair.first]->setName(pair.first->to_string().c_str());
    }

    auto const ret(gen(expr.body, arity));
    locals = std::move(old_locals);

    /* XXX: No return creation, since we rely on the body to do that. */

    return ret;
  }

  llvm::Value *llvm_processor::gen(expr::letfn<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto old_locals(locals);
    for(auto const &pair : expr.pairs)
    {
      auto const local(expr.frame->find_local_or_capture(pair.first));
      if(local.is_none())
      {
        throw std::runtime_error{ fmt::format("ICE: unable to find local: {}",
                                              pair.first->to_string()) };
      }

      locals[pair.first] = gen(pair.second, arity);
      locals[pair.first]->setName(pair.first->to_string().c_str());
    }

    auto const ret(gen(expr.body, arity));
    locals = std::move(old_locals);

    /* XXX: No return creation, since we rely on the body to do that. */

    return ret;
  }

  llvm::Value *llvm_processor::gen(expr::do_<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    llvm::Value *last{};
    for(auto const &form : expr.values)
    {
      last = gen(form, arity);
    }

    switch(expr.position)
    {
      case expression_position::statement:
      case expression_position::value:
        {
          return last;
        }
      case expression_position::tail:
        {
          if(expr.values.empty())
          {
            return ctx->builder->CreateRet(gen_global(obj::nil::nil_const()));
          }
          else
          {
            /* Codegen for this already generated a return. */
            return last;
          }
        }
    }
  }

  llvm::Value *llvm_processor::gen(expr::if_<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    /* If we're in return position, our then/else branches will generate return instructions
     * for us. Since LLVM basic blocks can only have one terminating instruction, we need
     * to take care to not generate our own, too. */
    auto const is_return(expr.position == expression_position::tail);
    auto const condition(gen(expr.condition, arity));
    auto const truthy_fn_type(
      llvm::FunctionType::get(ctx->builder->getInt8Ty(), { ctx->builder->getPtrTy() }, false));
    auto const fn(ctx->module->getOrInsertFunction("jank_truthy", truthy_fn_type));
    llvm::SmallVector<llvm::Value *, 1> const args{ condition };
    auto const call(ctx->builder->CreateCall(fn, args));
    auto const cmp(ctx->builder->CreateICmpEQ(call, ctx->builder->getInt8(1), "iftmp"));

    auto const current_fn(ctx->builder->GetInsertBlock()->getParent());
    auto then_block(llvm::BasicBlock::Create(*ctx->llvm_ctx, "then", current_fn));
    auto else_block(llvm::BasicBlock::Create(*ctx->llvm_ctx, "else"));
    auto const merge_block(llvm::BasicBlock::Create(*ctx->llvm_ctx, "ifcont"));

    ctx->builder->CreateCondBr(cmp, then_block, else_block);

    ctx->builder->SetInsertPoint(then_block);
    auto const then(gen(expr.then, arity));

    if(!is_return)
    {
      ctx->builder->CreateBr(merge_block);
    }

    /* Codegen for `then` can change the current block, so track that. */
    then_block = ctx->builder->GetInsertBlock();
    current_fn->insert(current_fn->end(), else_block);

    ctx->builder->SetInsertPoint(else_block);
    llvm::Value *else_{};

    if(expr.else_.is_some())
    {
      else_ = gen(expr.else_.unwrap(), arity);
    }
    else
    {
      else_ = gen_global(obj::nil::nil_const());
      if(expr.position == expression_position::tail)
      {
        else_ = ctx->builder->CreateRet(else_);
      }
    }

    if(!is_return)
    {
      ctx->builder->CreateBr(merge_block);
    }

    /* Codegen for `else` can change the current block, so track that. */
    else_block = ctx->builder->GetInsertBlock();

    if(!is_return)
    {
      current_fn->insert(current_fn->end(), merge_block);
      ctx->builder->SetInsertPoint(merge_block);
      auto const phi(
        ctx->builder->CreatePHI(is_return ? ctx->builder->getVoidTy() : ctx->builder->getPtrTy(),
                                2,
                                "iftmp"));
      phi->addIncoming(then, then_block);
      phi->addIncoming(else_, else_block);

      return phi;
    }
    return nullptr;
  }

  llvm::Value *llvm_processor::gen(expr::throw_<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    /* TODO: Generate direct call to __cxa_throw. */
    auto const value(gen(expr.value, arity));
    auto const fn_type(
      llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
    auto fn(ctx->module->getOrInsertFunction("jank_throw", fn_type));
    llvm::cast<llvm::Function>(fn.getCallee())->setDoesNotReturn();

    llvm::SmallVector<llvm::Value *, 1> const args{ value };
    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }
    return call;
  }

  llvm::Value *llvm_processor::gen(expr::try_<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const wrapped_body(
      evaluate::wrap_expression(make_box<expression>(expr.body), "try_body", {}));
    auto const wrapped_catch(expr.catch_body.map([](auto const &catch_body) {
      return evaluate::wrap_expression(make_box<expression>(catch_body.body),
                                       "catch",
                                       { catch_body.sym });
    }));
    auto const wrapped_finally(expr.finally_body.map([](auto const &finally) {
      return evaluate::wrap_expression(make_box<expression>(finally), "finally", {});
    }));

    auto const body(gen(wrapped_body, arity));
    auto const catch_(
      wrapped_catch.map([&](auto const &catch_body) { return gen(catch_body, arity); }));
    auto const finally(
      wrapped_finally.map([&](auto const &finally) { return gen(finally, arity); }));

    auto const fn_type(llvm::FunctionType::get(
      ctx->builder->getPtrTy(),
      { ctx->builder->getPtrTy(), ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
      false));
    auto const fn(ctx->module->getOrInsertFunction("jank_try", fn_type));

    llvm::SmallVector<llvm::Value *, 3> const args{
      body,
      catch_.unwrap_or(gen_global(obj::nil::nil_const())),
      finally.unwrap_or(gen_global(obj::nil::nil_const()))
    };
    auto const call(ctx->builder->CreateCall(fn, args));

    if(expr.position == expression_position::tail)
    {
      return ctx->builder->CreateRet(call);
    }
    return call;
  }

  llvm::Value *llvm_processor::gen(expr::case_<expression> const &expr,
                                   expr::function_arity<expression> const &arity)
  {
    auto const current_fn(ctx->builder->GetInsertBlock()->getParent());
    auto const position{ expr.position };
    auto const value(gen(expr.value_expr, arity));
    auto const is_return{ position == expression_position::tail };
    auto const integer_fn_type(llvm::FunctionType::get(
      ctx->builder->getInt64Ty(),
      { ctx->builder->getPtrTy(), ctx->builder->getInt64Ty(), ctx->builder->getInt64Ty() },
      false));
    auto const fn(
      ctx->module->getOrInsertFunction("jank_shift_mask_case_integer", integer_fn_type));
    llvm::SmallVector<llvm::Value *, 3> const args{
      value,
      llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), expr.shift),
      llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), expr.mask)
    };
    auto const call(ctx->builder->CreateCall(fn, args));
    auto const switch_val(ctx->builder->CreateIntCast(call, ctx->builder->getInt64Ty(), true));
    auto const default_block{ llvm::BasicBlock::Create(*ctx->llvm_ctx, "default", current_fn) };
    auto const switch_{ ctx->builder->CreateSwitch(switch_val, default_block, expr.keys.size()) };
    auto const merge_block{ is_return
                              ? nullptr
                              : llvm::BasicBlock::Create(*ctx->llvm_ctx, "merge", current_fn) };

    ctx->builder->SetInsertPoint(default_block);
    auto const default_val{ gen(expr.default_expr, arity) };
    if(!is_return)
    {
      ctx->builder->CreateBr(merge_block);
    }
    auto const default_block_exit{ ctx->builder->GetInsertBlock() };

    llvm::SmallVector<llvm::BasicBlock *> case_blocks;
    llvm::SmallVector<llvm::Value *> case_values;
    for(size_t block_counter{}; block_counter < expr.keys.size(); ++block_counter)
    {
      auto const block_name{ fmt::format("case_{}", block_counter) };
      auto const block{ llvm::BasicBlock::Create(*ctx->llvm_ctx, block_name, current_fn) };
      switch_->addCase(
        llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), expr.keys[block_counter]),
        block);

      ctx->builder->SetInsertPoint(block);
      auto const case_val{ gen(expr.exprs[block_counter], arity) };
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
        ctx->builder->CreatePHI(ctx->builder->getPtrTy(), expr.keys.size() + 1, "switch_tmp")
      };
      phi->addIncoming(default_val, default_block_exit);
      for(size_t i{}; i < case_blocks.size(); ++i)
      {
        phi->addIncoming(case_values[i], case_blocks[i]);
      }
      return phi;
    }
    return nullptr;
  }

  llvm::Value *llvm_processor::gen_var(obj::symbol_ptr const qualified_name) const
  {
    auto const found(ctx->var_globals.find(qualified_name));
    if(found != ctx->var_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->var_globals[qualified_name]);
    auto const name(fmt::format("var_{}", munge(qualified_name->to_string())));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);
      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const fn(ctx->module->getOrInsertFunction("jank_var_intern", fn_type));

      llvm::SmallVector<llvm::Value *, 2> const args{ gen_global(make_box(qualified_name->ns)),
                                                      gen_global(make_box(qualified_name->name)) };
      auto const call(ctx->builder->CreateCall(fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_c_string(native_persistent_string const &s) const
  {
    auto const found(ctx->c_string_globals.find(s));
    if(found != ctx->c_string_globals.end())
    {
      return found->second;
    }
    return ctx->c_string_globals[s] = ctx->builder->CreateGlobalStringPtr(s.c_str());
  }

  llvm::Value *llvm_processor::gen_global(obj::nil_ptr const nil) const
  {
    auto const found(ctx->literal_globals.find(nil));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[nil]);
    auto const name("nil");
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_nil", create_fn_type));
      auto const call(ctx->builder->CreateCall(create_fn));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::boolean_ptr const b) const
  {
    auto const found(ctx->literal_globals.find(b));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[b]);
    auto const name(b->data ? "true" : "false");
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(ctx->builder->getPtrTy(), false));
      auto const create_fn(
        ctx->module->getOrInsertFunction(fmt::format("jank_{}", name), create_fn_type));
      auto const call(ctx->builder->CreateCall(create_fn));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::integer_ptr const i) const
  {
    auto const found(ctx->literal_globals.find(i));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[i]);
    auto const name(fmt::format("int_{}", i->data));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_integer_create", create_fn_type));
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

  llvm::Value *llvm_processor::gen_global(obj::real_ptr const r) const
  {
    auto const found(ctx->literal_globals.find(r));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[r]);
    auto const name(fmt::format("real_{}", r->to_hash()));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getDoubleTy() }, false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_real_create", create_fn_type));
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

  llvm::Value *llvm_processor::gen_global(obj::ratio_ptr const r) const
  {
    if(auto const found(ctx->literal_globals.find(r)); found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[r]);
    auto const name(fmt::format("ratio_{}", r->to_hash()));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getInt64Ty(), ctx->builder->getInt64Ty() },
                                false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_ratio_create", create_fn_type));
      auto const num_arg(
        llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), r->data.numerator));
      auto const denom_arg(
        llvm::ConstantInt::getSigned(ctx->builder->getInt64Ty(), r->data.denominator));
      auto const call(ctx->builder->CreateCall(create_fn, { num_arg, denom_arg }));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::persistent_string_ptr const s) const
  {
    auto const found(ctx->literal_globals.find(s));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[s]);
    auto const name(fmt::format("string_{}", s->to_hash()));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_string_create", create_fn_type));

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

  llvm::Value *llvm_processor::gen_global(obj::symbol_ptr const s)
  {
    auto const found(ctx->literal_globals.find(s));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[s]);
    auto const name(fmt::format("symbol_{}", s->to_hash()));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_symbol_create", create_fn_type));

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
        auto const set_meta_fn(ctx->module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

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

  llvm::Value *llvm_processor::gen_global(obj::keyword_ptr const k) const
  {
    auto const found(ctx->literal_globals.find(k));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[k]);
    auto const name(fmt::format("keyword_{}", k->to_hash()));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_keyword_intern", create_fn_type));

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

  llvm::Value *llvm_processor::gen_global(obj::character_ptr const c) const
  {
    auto const found(ctx->literal_globals.find(c));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[c]);
    auto const name(fmt::format("char_{}", c->to_hash()));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(
        ctx->module->getOrInsertFunction("jank_character_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_c_string(c->to_string()) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      if(prev_block == ctx->global_ctor_block)
      {
        return call;
      }
    }

    return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global_from_read_string(object_ptr const o)
  {
    auto const found(ctx->literal_globals.find(o));
    if(found != ctx->literal_globals.end())
    {
      return ctx->builder->CreateLoad(ctx->builder->getPtrTy(), found->second);
    }

    auto &global(ctx->literal_globals[o]);
    auto const name(fmt::format("data_{}", to_hash(o)));
    auto const var(create_global_var(name));
    ctx->module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(ctx->builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getPtrTy() }, false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_read_string", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> const args{ gen_global(
        make_box(runtime::to_code_string(o))) };
      auto const call(ctx->builder->CreateCall(create_fn, args));
      ctx->builder->CreateStore(call, global);

      runtime::visit_object(
        [&](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(behavior::metadatable<T>)
          {
            if(typed_o->meta)
            {
              auto const set_meta_fn_type(
                llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                        { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                        false));
              auto const set_meta_fn(
                ctx->module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

              /* TODO: This shouldn't be its own global; we don't need to reference it later. */
              auto const meta(gen_global_from_read_string(typed_o->meta.unwrap()));
              auto const meta_name(fmt::format("{}_meta", name));
              meta->setName(meta_name);
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

  llvm::Value *
  llvm_processor::gen_function_instance(expr::function<expression> const &expr,
                                        expr::function_arity<expression> const &fn_arity)
  {
    expr::function_arity<expression> const *variadic_arity{};
    expr::function_arity<expression> const *highest_fixed_arity{};
    auto const captures(expr.captures());
    for(auto const &arity : expr.arities)
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
    auto const highest_fixed_args(variadic_arity ? variadic_arity->fn_ctx->param_count - 1
                                                 : highest_fixed_arity->fn_ctx->param_count);

    auto const arity_flags_fn_type(llvm::FunctionType::get(
      ctx->builder->getInt8Ty(),
      { ctx->builder->getInt8Ty(), ctx->builder->getInt8Ty(), ctx->builder->getInt8Ty() },
      false));
    auto const arity_flags_fn(
      ctx->module->getOrInsertFunction("jank_function_build_arity_flags", arity_flags_fn_type));
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
        ctx->module->getOrInsertFunction("jank_function_create", create_fn_type));
      fn_obj = ctx->builder->CreateCall(create_fn, { arity_flags });
    }
    else
    {
      std::vector<llvm::Type *> const capture_types{ captures.size(), ctx->builder->getPtrTy() };
      auto const closure_ctx_type(
        get_or_insert_struct_type(fmt::format("{}_context", munge(expr.unique_name)),
                                  capture_types));

      auto const malloc_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), { ctx->builder->getInt64Ty() }, false));
      auto const malloc_fn(ctx->module->getOrInsertFunction("GC_malloc", malloc_fn_type));
      auto const closure_obj(
        ctx->builder->CreateCall(malloc_fn, { llvm::ConstantExpr::getSizeOf(closure_ctx_type) }));

      size_t index{};
      for(auto const &capture : captures)
      {
        auto const field_ptr(ctx->builder->CreateStructGEP(closure_ctx_type, closure_obj, index++));
        expr::local_reference const local_ref{
          expression_base{ {}, expression_position::value, expr.frame },
          capture.first,
          *capture.second
        };
        ctx->builder->CreateStore(gen(local_ref, fn_arity), field_ptr);
      }

      auto const create_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(),
                                { ctx->builder->getInt8Ty(), ctx->builder->getPtrTy() },
                                false));
      auto const create_fn(ctx->module->getOrInsertFunction("jank_closure_create", create_fn_type));
      fn_obj = ctx->builder->CreateCall(create_fn, { arity_flags, closure_obj });
    }

    for(auto const &arity : expr.arities)
    {
      auto const set_arity_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_arity_fn(ctx->module->getOrInsertFunction(
        is_closure ? fmt::format("jank_closure_set_arity{}", arity.params.size())
                   : fmt::format("jank_function_set_arity{}", arity.params.size()),
        set_arity_fn_type));

      std::vector<llvm::Type *> const target_arg_types{ arity.params.size(),
                                                        ctx->builder->getPtrTy() };
      auto const target_fn_type(
        llvm::FunctionType::get(ctx->builder->getPtrTy(), target_arg_types, false));
      auto target_fn(ctx->module->getOrInsertFunction(
        fmt::format("{}_{}", munge(expr.unique_name), arity.params.size()),
        target_fn_type));

      ctx->builder->CreateCall(set_arity_fn, { fn_obj, target_fn.getCallee() });
    }

    if(expr.meta)
    {
      auto const set_meta_fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(),
                                { ctx->builder->getPtrTy(), ctx->builder->getPtrTy() },
                                false));
      auto const set_meta_fn(ctx->module->getOrInsertFunction("jank_set_meta", set_meta_fn_type));

      auto const meta(gen_global_from_read_string(expr.meta));
      ctx->builder->CreateCall(set_meta_fn, { fn_obj, meta });
    }

    return fn_obj;
  }

  void llvm_processor::create_global_ctor()
  {
    auto const init_type(llvm::FunctionType::get(ctx->builder->getVoidTy(), false));
    auto const init(llvm::Function::Create(init_type,
                                           llvm::Function::ExternalLinkage,
                                           ctx->ctor_name.c_str(),
                                           *ctx->module));
    ctx->global_ctor_block->insertInto(init);

    /* XXX: Modules are written to object files, which can't use global ctors until
     * we're on the ORC runtime. Instead, we just generate our load function to call
     * our global ctor first. */
    if(target != compilation_target::module)
    {
      llvm::appendToGlobalCtors(*ctx->module, init, 65535);
    }

    if(profile::is_enabled())
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *ctx->builder };
      ctx->builder->SetInsertPoint(ctx->global_ctor_block);

      auto const fn_type(
        llvm::FunctionType::get(ctx->builder->getVoidTy(), { ctx->builder->getPtrTy() }, false));
      auto const fn(ctx->module->getOrInsertFunction("jank_profile_enter", fn_type));
      ctx->builder->CreateCall(fn,
                               { gen_c_string(fmt::format("global ctor for {}", root_fn.name)) });
    }
  }

  llvm::GlobalVariable *
  llvm_processor::create_global_var(native_persistent_string const &name) const
  {
    return new llvm::GlobalVariable{ ctx->builder->getPtrTy(),
                                     false,
                                     llvm::GlobalVariable::InternalLinkage,
                                     llvm::ConstantPointerNull::get(ctx->builder->getPtrTy()),
                                     name.c_str() };
  }

  llvm::StructType *
  llvm_processor::get_or_insert_struct_type(std::string const &name,
                                            std::vector<llvm::Type *> const &fields) const
  {
    auto const found(llvm::StructType::getTypeByName(*ctx->llvm_ctx, name));
    if(found)
    {
      return found;
    }

    std::vector<llvm::Type *> const field_types{ fields.size(), ctx->builder->getPtrTy() };
    auto const struct_type(llvm::StructType::create(field_types, name));
    return struct_type;
  }

  native_persistent_string llvm_processor::to_string() const
  {
    ctx->module->print(llvm::outs(), nullptr);
    return "";
  }
}
