#include <llvm/IR/Verifier.h>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/codegen/llvm/processor.hpp>
#include <jank/codegen/escape.hpp>
#include <jank/detail/to_runtime_data.hpp>

#include <jit.hpp>

namespace jank::codegen::llvm
{
  processor::processor(runtime::context &rt_ctx,
                       analyze::expression_ptr const &expr,
                       native_persistent_string_view const &module,
                       compilation_target const target)
    : rt_ctx{ rt_ctx }
    , root_expr{ expr }
    , root_fn{ boost::get<analyze::expr::function<analyze::expression>>(expr->data) }
    , module{ module }
    , target{ target }
    , struct_name{ root_fn.name }
    , jit_context{ std::make_unique<::llvm::LLVMContext>() }
    , jit_module{ std::make_unique<::llvm::Module>("jank", *jit_context) }
    , jit_builder{ std::make_unique<::llvm::IRBuilder<>>(*jit_context) }
  {
    //jit_module->setDataLayout(rt_ctx.jit_prc.interpreter->getDataLayout());
    assert(root_fn.frame.data);
  }

  processor::processor(runtime::context &rt_ctx,
                       analyze::expr::function<analyze::expression> const &expr,
                       native_persistent_string_view const &module,
                       compilation_target const target)
    : rt_ctx{ rt_ctx }
    , root_fn{ expr }
    , module{ module }
    , target{ target }
    , struct_name{ root_fn.name }
    , jit_context{ std::make_unique<::llvm::LLVMContext>() }
    , jit_module{ std::make_unique<::llvm::Module>("jank", *jit_context) }
    , jit_builder{ std::make_unique<::llvm::IRBuilder<>>(*jit_context) }
  {
    //jit_module->setDataLayout(rt_ctx.jit_prc.interpreter->getDataLayout());
    assert(root_fn.frame.data);
  }

  ::llvm::Value *processor::gen(analyze::expression_ptr const &ex,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    ::llvm::Value *ret{};
    boost::apply_visitor(
      [this, fn_arity, &ret](auto const &typed_ex) { ret = gen(typed_ex, fn_arity); },
      ex->data);
    return ret;
  }

  ::llvm::Value *processor::gen(analyze::expr::def<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *LogErrorV(char const *Str)
  {
    fmt::println("error: {}", Str);
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::var_deref<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &)
  {
    ::llvm::Function *CalleeF = jit_module->getFunction("sin");
    if(!CalleeF)
    {
      return LogErrorV("unable to find fn: sin");
    }
    std::vector<::llvm::Value *> ArgsV;
    ArgsV.push_back(::llvm::ConstantFP::get(*jit_context, ::llvm::APFloat(44.0)));

    return jit_builder->CreateCall(CalleeF, ArgsV, "calltmp");
  }

  ::llvm::Value *processor::gen(analyze::expr::var_ref<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::call<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::vector<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::map<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::set<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::local_reference const &expr,
                                analyze::expr::function_arity<analyze::expression> const &)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::function<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &)
  {
    std::vector<::llvm::Type *> arg_types(expr.arities[0].params.size(),
                                          ::llvm::Type::getInt8PtrTy(*jit_context));
    ::llvm::FunctionType *ft
      = ::llvm::FunctionType::get(::llvm::Type::getInt8PtrTy(*jit_context), arg_types, false);

    ::llvm::Function *fn = ::llvm::Function::Create(ft,
                                                    ::llvm::Function::ExternalLinkage,
                                                    expr.name.c_str(),
                                                    jit_module.get());

    size_t arg_index{};
    for(auto &arg : fn->args())
    {
      arg.setName(expr.arities[0].params[arg_index++]->name.c_str());
    }

    ::llvm::BasicBlock *bb = ::llvm::BasicBlock::Create(*jit_context, "entry", fn);
    jit_builder->SetInsertPoint(bb);

    jit_builder->CreateRet(gen(expr.arities[0].body.body[0], expr.arities[0]));

    ::llvm::verifyFunction(*fn);
    fmt::println("BINGBONG build fn");

    return fn;
  }

  ::llvm::Value *processor::gen(analyze::expr::recur<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::let<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::do_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::if_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::throw_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::try_<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  ::llvm::Value *processor::gen(analyze::expr::native_raw<analyze::expression> const &expr,
                                analyze::expr::function_arity<analyze::expression> const &fn_arity)
  {
    return nullptr;
  }

  void processor::gen_body()
  {
    for(auto const &form : root_fn.arities[0].body.body)
    {
      gen(form, root_fn.arities[0]);
    }
  }
}
