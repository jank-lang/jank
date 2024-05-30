#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
//#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>

namespace llvm::orc
{
  class KaleidoscopeJIT
  {
  private:
    std::unique_ptr<ExecutionSession> ES;

    DataLayout DL;
    MangleAndInterner Mangle;

    RTDyldObjectLinkingLayer ObjectLayer;
    IRCompileLayer CompileLayer;

    JITDylib &MainJD;

  public:
    KaleidoscopeJIT(std::unique_ptr<ExecutionSession> ES_,
                    JITTargetMachineBuilder JTMB_,
                    DataLayout DL_)
      : ES(std::move(ES_))
      , DL(std::move(DL_))
      , Mangle(*this->ES, this->DL)
      , ObjectLayer(*this->ES, []() { return std::make_unique<SectionMemoryManager>(); })
      , CompileLayer(*this->ES,
                     ObjectLayer,
                     std::make_unique<ConcurrentIRCompiler>(std::move(JTMB_)))
      , MainJD(this->ES->createBareJITDylib("<main>"))
    {
      MainJD.addGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix())));
    }

    ~KaleidoscopeJIT()
    {
      if(auto Err = ES->endSession())
      {
        ES->reportError(std::move(Err));
      }
    }

    static Expected<std::unique_ptr<KaleidoscopeJIT>> Create()
    {
      auto EPC = SelfExecutorProcessControl::Create();
      if(!EPC)
      {
        return EPC.takeError();
      }

      auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

      JITTargetMachineBuilder JTMB(ES->getExecutorProcessControl().getTargetTriple());

      auto DL = JTMB.getDefaultDataLayoutForTarget();
      if(!DL)
      {
        return DL.takeError();
      }

      return std::make_unique<KaleidoscopeJIT>(std::move(ES), std::move(JTMB), std::move(*DL));
    }

    DataLayout const &getDataLayout() const
    {
      return DL;
    }

    JITDylib &getMainJITDylib()
    {
      return MainJD;
    }

    Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr)
    {
      if(!RT)
      {
        RT = MainJD.getDefaultResourceTracker();
      }
      return CompileLayer.add(RT, std::move(TSM));
    }

    auto lookup(StringRef Name)
    {
      return ES->lookup({ &MainJD }, Mangle(Name.str()));
    }
  };

} // end namespace llvm::orc
