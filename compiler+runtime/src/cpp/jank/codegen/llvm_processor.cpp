#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <jank/codegen/llvm_processor.hpp>

namespace jank::codegen
{
  llvm_processor::llvm_processor(analyze::expression_ptr const &expr,
                                 native_persistent_string const &module_name,
                                 compilation_target const target)
    : llvm_processor{ boost::get<analyze::expr::function<analyze::expression>>(expr->data),
                      module_name,
                      target }
  {
  }

  llvm_processor::llvm_processor(analyze::expr::function<analyze::expression> const &expr,
                                 native_persistent_string const &module_name,
                                 compilation_target const target)
    : root_fn{ expr }
    , module_name{ module_name }
    , target{ target }
    , struct_name{ runtime::munge(root_fn.unique_name) }
    , ctor_name{ runtime::munge(runtime::context::unique_string("jank_global_init")) }
    , context{ std::make_unique<llvm::LLVMContext>() }
    , module{ std::make_unique<llvm::Module>(runtime::context::unique_string(module_name).c_str(),
                                             *context) }
    , builder{ std::make_unique<llvm::IRBuilder<>>(*context) }
    , global_ctor_block{ llvm::BasicBlock::Create(*context, "entry") }
  {
    assert(root_fn.frame.data);
  }

  llvm_processor::llvm_processor(nested_tag,
                                 analyze::expr::function<analyze::expression> const &expr,
                                 llvm_processor &&from)
    : root_fn{ expr }
    , module_name(from.module_name)
    , target{ compilation_target::function }
    , struct_name{ runtime::munge(root_fn.unique_name) }
    , ctor_name{ runtime::munge(runtime::context::unique_string("jank_global_init")) }
    , context{ std::move(from.context) }
    , module{ std::move(from.module) }
    , builder{ std::move(from.builder) }
    , global_ctor_block{ from.global_ctor_block }
  {
  }

  void llvm_processor::release(llvm_processor &into) &&
  {
    into.context = std::move(context);
    into.module = std::move(module);
    into.builder = std::move(builder);
  }

  void llvm_processor::create_function()
  {
    auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(), false));
    fn = llvm::Function::Create(fn_type,
                                llvm::Function::ExternalLinkage,
                                struct_name.c_str(),
                                *module);

    auto const entry(llvm::BasicBlock::Create(*context, "entry", fn));
    builder->SetInsertPoint(entry);
  }

  void
  llvm_processor::create_function(analyze::expr::function_arity<analyze::expression> const &arity)
  {
    auto const captures(root_fn.captures());
    auto const is_closure(!captures.empty());
    fmt::println("Creating fn {} with arities: {} captures: {}",
                 root_fn.unique_name,
                 root_fn.arities.size(),
                 captures.size());

    /* Closures get one extra parameter, the first one, which is a pointer to the closure's
     * context. The context is a struct containing all captured values. */
    std::vector<llvm::Type *> const arg_types{ arity.params.size() + is_closure,
                                               builder->getPtrTy() };
    auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(), arg_types, false));
    fn = llvm::Function::Create(fn_type,
                                llvm::Function::ExternalLinkage,
                                fmt::format("{}_{}", struct_name, arity.params.size()),
                                *module);

    auto const entry(llvm::BasicBlock::Create(*context, "entry", fn));
    builder->SetInsertPoint(entry);

    for(size_t i{}; i < arity.params.size(); ++i)
    {
      locals[arity.params[i]] = fn->getArg(i + is_closure);
    }

    if(is_closure)
    {
      auto const context(fn->getArg(0));
      for(auto const &capture : arity.frame->captures)
      {
        auto const captures(root_fn.captures());
        std::vector<llvm::Type *> const capture_types{ captures.size(), builder->getPtrTy() };
        auto const closure_ctx_type(
          get_or_insert_struct_type(fmt::format("{}_context", munge(root_fn.unique_name)),
                                    capture_types));
        auto const field_ptr(builder->CreateStructGEP(closure_ctx_type, context, 0));
        locals[capture.first] = builder->CreateLoad(builder->getPtrTy(),
                                                    field_ptr,
                                                    munge(capture.first->to_string()).c_str());
      }
    }
  }

  void llvm_processor::gen()
  {
    if(target == compilation_target::repl)
    {
      create_global_ctor();
    }

    for(auto const &arity : root_fn.arities)
    {
      create_function(arity);
      for(auto const &form : arity.body.values)
      {
        gen(form, arity);
      }
    }

    if(target != compilation_target::function)
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);
      builder->CreateRetVoid();

      llvm::verifyFunction(*fn);
      llvm::verifyFunction(*global_ctor_block->getParent());
    }
  }

  llvm::Value *
  llvm_processor::gen(analyze::expression_ptr const &ex,
                      analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    llvm::Value *ret{};
    boost::apply_visitor(
      [this, fn_arity, &ret](auto const &typed_ex) { ret = gen(typed_ex, fn_arity); },
      ex->data);
    return ret;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::def<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &arity)
  {
    auto const ref(gen_var(expr.name));

    if(expr.value.is_some())
    {
      auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(),
                                                 { builder->getPtrTy(), builder->getPtrTy() },
                                                 false));
      auto const fn(module->getOrInsertFunction("jank_var_bind_root", fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ ref, gen(expr.value.unwrap(), arity) };
      builder->CreateCall(fn, args);
    }

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ref);
    }

    return ref;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::var_deref<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const ref(gen_var(expr.qualified_name));
    auto const fn_type(
      llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
    auto const fn(module->getOrInsertFunction("jank_deref", fn_type));

    llvm::SmallVector<llvm::Value *, 1> args{ ref };
    auto const call(builder->CreateCall(fn, args));

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::var_ref<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const var(gen_var(expr.qualified_name));

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(var);
    }

    return var;
  }

  static native_persistent_string arity_to_call_fn(size_t const arity)
  {
    switch(arity)
    {
      case 0 ... 10:
        return fmt::format("jank_call{}", arity);
      default:
        throw std::runtime_error{ fmt::format("invalid fn arity: {}", arity) };
    }
  }

  llvm::Value *llvm_processor::gen(analyze::expr::call<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &arity)
  {
    auto const callee(gen(expr.source_expr, arity));

    llvm::SmallVector<llvm::Value *> arg_handles;
    llvm::SmallVector<llvm::Type *> arg_types;
    arg_handles.reserve(expr.arg_exprs.size() + 1);
    arg_types.reserve(expr.arg_exprs.size() + 1);

    arg_handles.emplace_back(callee);
    arg_types.emplace_back(builder->getPtrTy());

    for(auto const &arg_expr : expr.arg_exprs)
    {
      arg_handles.emplace_back(gen(arg_expr, arity));
      arg_types.emplace_back(builder->getPtrTy());
    }

    auto const call_fn_name(arity_to_call_fn(expr.arg_exprs.size()));

    auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(), arg_types, false));
    auto const fn(module->getOrInsertFunction(call_fn_name.c_str(), fn_type));
    auto const call(builder->CreateCall(fn, arg_handles));

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(call);
    }

    return call;
  }

  llvm::Value *
  llvm_processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr,
                      analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const ret(runtime::visit_object(
      [&](auto const typed_o) -> llvm::Value * {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, runtime::obj::nil> || std::same_as<T, runtime::obj::boolean>
                     || std::same_as<T, runtime::obj::integer>
                     || std::same_as<T, runtime::obj::real> || std::same_as<T, runtime::obj::symbol>
                     || std::same_as<T, runtime::obj::character>
                     || std::same_as<T, runtime::obj::keyword>
                     || std::same_as<T, runtime::obj::persistent_string>)
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

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::vector<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const ret(gen_global_from_read_string(expr.data));

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::map<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const ret(gen_global_from_read_string(expr.data));

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::set<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const ret(gen_global_from_read_string(expr.data));

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::local_reference const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    auto const ret(locals[expr.binding.name]);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *
  llvm_processor::gen(analyze::expr::function<analyze::expression> const &expr,
                      analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };

      llvm_processor nested{ nested_tag{}, expr, std::move(*this) };
      nested.gen();

      std::move(nested).release(*this);
    }

    analyze::expr::function_arity<analyze::expression> const *variadic_arity{};
    analyze::expr::function_arity<analyze::expression> const *highest_fixed_arity{};
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
    native_bool const variadic_ambiguous{ highest_fixed_arity && variadic_arity
                                          && highest_fixed_arity->fn_ctx->param_count
                                            == variadic_arity->fn_ctx->param_count - 1 };

    auto const arity_flags_fn_type(
      llvm::FunctionType::get(builder->getInt8Ty(),
                              { builder->getInt8Ty(), builder->getInt8Ty(), builder->getInt8Ty() },
                              false));
    auto const arity_flags_fn(
      module->getOrInsertFunction("jank_function_build_arity_flags", arity_flags_fn_type));
    auto const arity_flags(builder->CreateCall(
      arity_flags_fn,
      { builder->getInt8(highest_fixed_arity ? highest_fixed_arity->params.size() : 0),
        builder->getInt8(!!variadic_arity),
        builder->getInt8(variadic_ambiguous) }));

    llvm::Value *fn_obj{};

    if(captures.empty())
    {
      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getInt8Ty() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_function_create", create_fn_type));
      fn_obj = builder->CreateCall(create_fn, { arity_flags });
    }
    else
    {
      std::vector<llvm::Type *> const capture_types{ captures.size(), builder->getPtrTy() };
      auto const closure_ctx_type(
        get_or_insert_struct_type(fmt::format("{}_context", munge(expr.unique_name)),
                                  capture_types));

      auto const malloc_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getInt64Ty() }, false));
      auto const malloc_fn(module->getOrInsertFunction("malloc", malloc_fn_type));
      auto const closure_obj(
        builder->CreateCall(malloc_fn, { llvm::ConstantExpr::getSizeOf(closure_ctx_type) }));

      for(auto const &capture : captures)
      {
        auto const field_ptr(builder->CreateStructGEP(closure_ctx_type, closure_obj, 0));
        analyze::expr::local_reference const local_ref{
          analyze::expression_base{ {}, expr.expr_type, expr.frame },
          capture.first,
          *capture.second
        };
        builder->CreateStore(gen(local_ref, fn_arity), field_ptr);
      }

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(),
                                { builder->getInt8Ty(), builder->getPtrTy() },
                                false));
      auto const create_fn(
        module->getOrInsertFunction("jank_function_create_closure", create_fn_type));
      fn_obj = builder->CreateCall(create_fn, { arity_flags, closure_obj });
    }

    for(auto const &arity : expr.arities)
    {
      std::vector<llvm::Type *> const arg_types{ arity.params.size() + 1, builder->getPtrTy() };
      auto const set_arity_fn_type(llvm::FunctionType::get(builder->getPtrTy(), arg_types, false));
      auto const set_arity_fn(
        module->getOrInsertFunction(fmt::format("jank_function_set_arity{}", arity.params.size()),
                                    set_arity_fn_type));

      std::vector<llvm::Type *> const target_arg_types{ arity.params.size(), builder->getPtrTy() };
      auto const target_fn_type(llvm::FunctionType::get(builder->getPtrTy(), arg_types, false));
      auto target_fn(module->getOrInsertFunction(
        fmt::format("{}_{}", munge(expr.unique_name), arity.params.size()),
        target_fn_type));

      builder->CreateCall(set_arity_fn, { fn_obj, target_fn.getCallee() });
    }

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(fn_obj);
    }

    return fn_obj;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::recur<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(nullptr);
    }

    return nullptr;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::let<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &arity)
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
    }

    auto const ret(gen(expr.body, arity));
    locals = std::move(old_locals);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(ret);
    }

    return ret;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::do_<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &arity)
  {
    llvm::Value *last{};
    for(auto const &form : expr.values)
    {
      last = gen(form, arity);
    }

    switch(expr.expr_type)
    {
      case analyze::expression_type::statement:
      case analyze::expression_type::nested:
        {
          return last;
        }
      case analyze::expression_type::return_statement:
        {
          if(!last)
          {
            return builder->CreateRet(gen_global(obj::nil::nil_const()));
          }
          else
          {
            return builder->CreateRet(last);
          }
        }
    }
  }

  llvm::Value *llvm_processor::gen(analyze::expr::if_<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &arity)
  {
    auto const condition(gen(expr.condition, arity));
    auto const truthy_fn_type(
      llvm::FunctionType::get(builder->getInt8Ty(), { builder->getPtrTy() }, false));
    auto const fn(module->getOrInsertFunction("jank_truthy", truthy_fn_type));
    llvm::SmallVector<llvm::Value *, 1> args{ condition };
    auto const call(builder->CreateCall(fn, args));
    auto const cmp(builder->CreateICmpEQ(call, builder->getInt8(1), "iftmp"));

    auto const current_fn(builder->GetInsertBlock()->getParent());
    auto then_block(llvm::BasicBlock::Create(*context, "then", current_fn));
    auto else_block(llvm::BasicBlock::Create(*context, "else"));
    auto const merge_block(llvm::BasicBlock::Create(*context, "ifcont"));

    builder->CreateCondBr(cmp, then_block, else_block);
    builder->SetInsertPoint(then_block);

    auto const then(gen(expr.then, arity));
    builder->CreateBr(merge_block);
    /* Codegen for `then` can change the current block, so track that. */
    then_block = builder->GetInsertBlock();
    current_fn->insert(current_fn->end(), else_block);
    builder->SetInsertPoint(else_block);

    auto const else_(gen(expr.else_.unwrap(), arity));
    builder->CreateBr(merge_block);
    /* Codegen for `else` can change the current block, so track that. */
    else_block = builder->GetInsertBlock();
    current_fn->insert(current_fn->end(), merge_block);

    builder->SetInsertPoint(merge_block);
    auto const phi(builder->CreatePHI(builder->getPtrTy(), 2, "iftmp"));
    phi->addIncoming(then, then_block);
    phi->addIncoming(else_, else_block);

    if(expr.expr_type == analyze::expression_type::return_statement)
    {
      return builder->CreateRet(phi);
    }

    return phi;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::throw_<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &arity)
  {
    auto const value(gen(expr.value, arity));
    auto const fn_type(
      llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
    auto const fn(module->getOrInsertFunction("jank_throw", fn_type));

    llvm::SmallVector<llvm::Value *, 2> args{ value };
    builder->CreateCall(fn, args);
    return builder->CreateUnreachable();
  }

  llvm::Value *llvm_processor::gen(analyze::expr::try_<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::native_raw<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  llvm::Value *llvm_processor::gen_var(obj::symbol_ptr const qualified_name)
  {
    auto const found(var_globals.find(qualified_name));
    if(found != var_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[qualified_name]);
    auto const name(fmt::format("var_{}", munge(qualified_name->to_string())));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);
      auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(),
                                                 { builder->getPtrTy(), builder->getPtrTy() },
                                                 false));
      auto const fn(module->getOrInsertFunction("jank_var_intern", fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ gen_global(make_box(qualified_name->ns)),
                                                gen_global(make_box(qualified_name->name)) };
      auto const call(builder->CreateCall(fn, args));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_c_string(native_persistent_string const &s)
  {
    auto const found(c_string_globals.find(s));
    if(found != c_string_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }
    return c_string_globals[s] = builder->CreateGlobalStringPtr(s.c_str());
  }

  llvm::Value *llvm_processor::gen_global(obj::nil_ptr const nil)
  {
    auto const found(literal_globals.find(nil));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[nil]);
    auto const name("nil");
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(builder->getPtrTy(), false));
      auto const create_fn(module->getOrInsertFunction("jank_nil", create_fn_type));
      auto const call(builder->CreateCall(create_fn));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::boolean_ptr const b)
  {
    auto const found(literal_globals.find(b));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[b]);
    auto const name(b->data ? "true" : "false");
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(builder->getPtrTy(), false));
      auto const create_fn(
        module->getOrInsertFunction(fmt::format("jank_{}", name), create_fn_type));
      auto const call(builder->CreateCall(create_fn));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::integer_ptr const i)
  {
    auto const found(literal_globals.find(i));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[i]);
    auto const name(fmt::format("int_{}", i->data));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_integer_create", create_fn_type));
      auto const arg(llvm::ConstantInt::getSigned(builder->getInt64Ty(), i->data));
      auto const call(builder->CreateCall(create_fn, { arg }));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::real_ptr const r)
  {
    auto const found(literal_globals.find(r));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[r]);
    auto const name(fmt::format("real_{}", r->to_hash()));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_integer_create", create_fn_type));
      auto const arg(llvm::ConstantFP::get(builder->getDoubleTy(), r->data));
      auto const call(builder->CreateCall(create_fn, { arg }));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::persistent_string_ptr const s)
  {
    auto const found(literal_globals.find(s));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[s]);
    auto const name(fmt::format("string_{}", s->to_hash()));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_string_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> args{ gen_c_string(s->data.c_str()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::symbol_ptr const s)
  {
    auto const found(literal_globals.find(s));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[s]);
    auto const name(fmt::format("symbol_{}", s->to_hash()));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(),
                                { builder->getPtrTy(), builder->getPtrTy() },
                                false));
      auto const create_fn(module->getOrInsertFunction("jank_symbol_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ gen_global(make_box(s->ns)),
                                                gen_global(make_box(s->name)) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::keyword_ptr const k)
  {
    auto const found(literal_globals.find(k));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[k]);
    auto const name(fmt::format("keyword_{}", k->to_hash()));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(),
                                { builder->getPtrTy(), builder->getPtrTy() },
                                false));
      auto const create_fn(module->getOrInsertFunction("jank_keyword_intern", create_fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ gen_c_string(k->sym.ns.c_str()),
                                                gen_c_string(k->sym.name.c_str()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global(obj::character_ptr const c)
  {
    auto const found(literal_globals.find(c));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[c]);
    auto const name(fmt::format("char_{}", c->to_hash()));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_character_create", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> args{ gen_c_string(c->to_string()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  llvm::Value *llvm_processor::gen_global_from_read_string(object_ptr const o)
  {
    auto const found(literal_globals.find(o));
    if(found != literal_globals.end())
    {
      return builder->CreateLoad(builder->getPtrTy(), found->second);
    }

    auto &global(literal_globals[o]);
    auto const name(fmt::format("data_{}", to_hash(o)));
    auto const var(create_global_var(name));
    module->insertGlobalVariable(var);
    global = var;

    auto const prev_block(builder->GetInsertBlock());
    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_read_string", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> args{ gen_global(make_box(runtime::to_code_string(o))) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);

      if(prev_block == global_ctor_block)
      {
        return call;
      }
    }

    return builder->CreateLoad(builder->getPtrTy(), global);
  }

  void llvm_processor::create_global_ctor()
  {
    auto const init_type(llvm::FunctionType::get(builder->getVoidTy(), false));
    auto const init(llvm::Function::Create(init_type,
                                           llvm::Function::ExternalLinkage,
                                           ctor_name.c_str(),
                                           *module));
    global_ctor_block->insertInto(init);

    llvm::appendToGlobalCtors(*module, init, 65535);
  }

  llvm::GlobalVariable *llvm_processor::create_global_var(native_persistent_string const &name)
  {
    return new llvm::GlobalVariable{ builder->getPtrTy(),
                                     false,
                                     llvm::GlobalVariable::InternalLinkage,
                                     builder->getInt64(0),
                                     name.c_str() };
  }

  llvm::StructType *
  llvm_processor::get_or_insert_struct_type(std::string const &name,
                                            std::vector<llvm::Type *> const &fields)
  {
    auto const found(llvm::StructType::getTypeByName(*context, name));
    if(found)
    {
      return found;
    }

    std::vector<llvm::Type *> const field_types{ fields.size(), builder->getPtrTy() };
    auto const struct_type(llvm::StructType::create(field_types, name));
    return struct_type;
  }

  native_persistent_string llvm_processor::to_string()
  {
    module->print(llvm::outs(), nullptr);
    return "";
  }
}
