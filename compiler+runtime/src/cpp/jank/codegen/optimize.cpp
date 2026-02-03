#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Transforms/IPO/Internalize.h>
#include <llvm/Transforms/IPO/GlobalDCE.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include <jank/runtime/module/loader.hpp>
#include <jank/codegen/optimize.hpp>
#include <jank/util/cli.hpp>

namespace jank::codegen
{
  void optimize(jtl::ref<llvm::Module> const module, jtl::immutable_string const &module_name)
  {
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    llvm::PassInstrumentationCallbacks pic;
    llvm::StandardInstrumentations si{ module->getContext(), /*DebugLogging*/ false };
    llvm::ModulePassManager mpm;

    /* TODO: Add more passes and measure the order of the passes. */

    si.registerCallbacks(pic, &mam);

    llvm::PassBuilder pb;
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    auto level{ llvm::OptimizationLevel::O2 };
    switch(util::cli::opts.optimization_level)
    {
      case 0:
        level = llvm::OptimizationLevel::O0;
        break;
      case 1:
        level = llvm::OptimizationLevel::O1;
        break;
      case 2:
        level = llvm::OptimizationLevel::O2;
        break;
      case 3:
        level = llvm::OptimizationLevel::O3;
        break;
      default:
        break;
    }

    mpm = pb.buildPerModuleDefaultPipeline(level);

    std::string const load_fn{ runtime::module::module_to_load_function(module_name) };
    auto const preserve{ [&](llvm::GlobalValue const &GV) { return GV.getName() == load_fn; } };

    mpm.addPass(llvm::InternalizePass(preserve));
    mpm.addPass(llvm::GlobalDCEPass());

    mpm.run(*module, mam);
  }
}
