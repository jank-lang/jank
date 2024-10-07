#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <jank/codegen/llvm_processor.hpp>

/* https://www.youtube.com/watch?v=Nw9YmNuJhJ4 */

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
    , struct_name{ root_fn.unique_name }
    , context{ std::make_unique<llvm::LLVMContext>() }
    , module{ std::make_unique<llvm::Module>(module_name.c_str(), *context) }
    , builder{ std::make_unique<llvm::IRBuilder<>>(*context) }
    , global_ctor_block{ llvm::BasicBlock::Create(*context, "entry") }
  {
    assert(root_fn.frame.data);

    create_global_ctor();
    create_function();
    gen();

    llvm::verifyFunction(*fn);
    llvm::verifyFunction(*global_ctor_block->getParent());
  }

  void llvm_processor::create_function()
  {
    auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(), false));
    auto const munged_name(runtime::munge(struct_name));
    fn = llvm::Function::Create(fn_type,
                                llvm::Function::ExternalLinkage,
                                munged_name.c_str(),
                                *module);

    auto const entry(llvm::BasicBlock::Create(*context, "entry", fn));
    builder->SetInsertPoint(entry);
  }

  void llvm_processor::gen()
  {
    auto const &arity(root_fn.arities[0]);
    for(auto const &form : arity.body.values)
    {
      gen(form, arity);
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
    return call;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::var_ref<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return gen_var(expr.qualified_name);
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

    return call;
  }

  llvm::Value *
  llvm_processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr,
                      analyze::expr::function_arity<analyze::expression> const &)
  {
    return runtime::visit_object(
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
      expr.data);
  }

  llvm::Value *llvm_processor::gen(analyze::expr::vector<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return gen_global_from_read_string(expr.data);
  }

  llvm::Value *llvm_processor::gen(analyze::expr::map<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return gen_global_from_read_string(expr.data);
  }

  llvm::Value *llvm_processor::gen(analyze::expr::set<analyze::expression> const &expr,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return gen_global_from_read_string(expr.data);
  }

  llvm::Value *llvm_processor::gen(analyze::expr::local_reference const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::function<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::recur<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::let<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
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

  llvm::Value *llvm_processor::gen(analyze::expr::if_<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  llvm::Value *llvm_processor::gen(analyze::expr::throw_<analyze::expression> const &,
                                   analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
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
      return found->second;
    }

    auto const name(fmt::format("var_{}", munge(qualified_name->to_string())));
    auto const global(module->getOrInsertGlobal(name, builder->getPtrTy()));
    var_globals[qualified_name] = global;

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);
      auto const fn_type(llvm::FunctionType::get(builder->getPtrTy(),
                                                 { builder->getPtrTy(), builder->getPtrTy() },
                                                 false));
      auto const fn(module->getOrInsertFunction("jank_var_intern", fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ gen_c_string(qualified_name->ns.c_str()),
                                                gen_c_string(qualified_name->name.c_str()) };
      auto const call(builder->CreateCall(fn, args));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_c_string(native_persistent_string const &s)
  {
    auto const found(c_string_globals.find(s));
    if(found != c_string_globals.end())
    {
      return found->second;
    }
    return c_string_globals[s] = builder->CreateGlobalStringPtr(s.c_str());
  }

  llvm::Value *llvm_processor::gen_global(obj::nil_ptr const nil)
  {
    auto const found(literal_globals.find(nil));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[nil]);
    global = module->getOrInsertGlobal("nil", builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(builder->getPtrTy(), false));
      auto const create_fn(module->getOrInsertFunction("jank_nil", create_fn_type));
      auto const call(builder->CreateCall(create_fn));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::boolean_ptr const b)
  {
    auto const found(literal_globals.find(b));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[b]);
    auto const name(b->data ? "true" : "false");
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(llvm::FunctionType::get(builder->getPtrTy(), false));
      auto const create_fn(
        module->getOrInsertFunction(fmt::format("jank_{}", name), create_fn_type));
      auto const call(builder->CreateCall(create_fn));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::integer_ptr const i)
  {
    auto const found(literal_globals.find(i));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[i]);
    auto const name(fmt::format("int_{}", i->data));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_create_integer", create_fn_type));
      auto const arg(llvm::ConstantInt::getSigned(builder->getInt64Ty(), i->data));
      auto const call(builder->CreateCall(create_fn, { arg }));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::real_ptr const r)
  {
    auto const found(literal_globals.find(r));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[r]);
    auto const name(fmt::format("real_{}", r->to_hash()));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_create_integer", create_fn_type));
      auto const arg(llvm::ConstantFP::get(builder->getDoubleTy(), r->data));
      auto const call(builder->CreateCall(create_fn, { arg }));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::persistent_string_ptr const s)
  {
    auto const found(literal_globals.find(s));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[s]);
    auto const name(fmt::format("string_{}", s->to_hash()));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_create_string", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> args{ gen_c_string(s->data.c_str()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::symbol_ptr const s)
  {
    auto const found(literal_globals.find(s));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[s]);
    auto const name(fmt::format("symbol_{}", s->to_hash()));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(),
                                { builder->getPtrTy(), builder->getPtrTy() },
                                false));
      auto const create_fn(module->getOrInsertFunction("jank_create_symbol", create_fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ gen_c_string(s->ns.c_str()),
                                                gen_c_string(s->name.c_str()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::keyword_ptr const k)
  {
    auto const found(literal_globals.find(k));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[k]);
    auto const name(fmt::format("keyword_{}", k->to_hash()));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(),
                                { builder->getPtrTy(), builder->getPtrTy() },
                                false));
      auto const create_fn(module->getOrInsertFunction("jank_create_keyword", create_fn_type));

      llvm::SmallVector<llvm::Value *, 2> args{ gen_c_string(k->sym.ns.c_str()),
                                                gen_c_string(k->sym.name.c_str()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global(obj::character_ptr const c)
  {
    auto const found(literal_globals.find(c));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[c]);
    auto const name(fmt::format("char_{}", c->to_hash()));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_create_character", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> args{ gen_c_string(c->to_string()) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);
    }

    return global;
  }

  llvm::Value *llvm_processor::gen_global_from_read_string(object_ptr const o)
  {
    auto const found(literal_globals.find(o));
    if(found != literal_globals.end())
    {
      return found->second;
    }

    auto &global(literal_globals[o]);
    auto const name(fmt::format("data_{}", to_hash(o)));
    global = module->getOrInsertGlobal(name, builder->getPtrTy());

    {
      llvm::IRBuilder<>::InsertPointGuard const guard{ *builder };
      builder->SetInsertPoint(global_ctor_block);

      auto const create_fn_type(
        llvm::FunctionType::get(builder->getPtrTy(), { builder->getPtrTy() }, false));
      auto const create_fn(module->getOrInsertFunction("jank_read_string", create_fn_type));

      llvm::SmallVector<llvm::Value *, 1> args{ gen_c_string(runtime::to_string(o)) };
      auto const call(builder->CreateCall(create_fn, args));
      builder->CreateStore(call, global);
    }

    return global;
  }

  void llvm_processor::create_global_ctor()
  {
    llvm::IRBuilder<> builder{ *context };
    auto const init_type(llvm::FunctionType::get(builder.getVoidTy(), false));
    auto const init(llvm::Function::Create(init_type,
                                           llvm::Function::InternalLinkage,
                                           "jank_global_init",
                                           *module));
    global_ctor_block->insertInto(init);

    llvm::appendToGlobalCtors(*module, init, 65535);
  }

  native_persistent_string llvm_processor::to_string()
  {
    module->print(llvm::outs(), nullptr);
    return "";
  }
}
