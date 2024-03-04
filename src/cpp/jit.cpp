//#include <jit.hpp>
//
//#include <llvm/IR/Module.h>
//#include <llvm/IRReader/IRReader.h>
//#include <llvm/Support/SourceMgr.h>
//
//int main()
//{
//  LLVMInitializeNativeTarget();
//  LLVMInitializeNativeAsmPrinter();
//  LLVMInitializeNativeAsmParser();
//
//  auto TheJIT = llvm::cantFail(llvm::orc::KaleidoscopeJIT::Create());
//
//  for(auto const file :
//      { std::make_pair("example.ll", "entry"), std::make_pair("example2.ll", "entry2") })
//  {
//    auto TheContext = std::make_unique<llvm::LLVMContext>();
//    //auto TheModule = std::make_unique<llvm::Module>("my cool jit", *TheContext);
//    //TheModule->setDataLayout(TheJIT->getDataLayout());
//    //auto Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
//
//    llvm::SMDiagnostic error;
//    std::unique_ptr<llvm::Module> module = llvm::parseIRFile(file.first, error, *TheContext);
//    llvm::cantFail(
//      TheJIT->addModule(llvm::orc::ThreadSafeModule(std::move(module), std::move(TheContext))));
//
//    auto entry = llvm::cantFail(TheJIT->lookup(file.second));
//    auto fp = (int (*)(int))(intptr_t)entry.getAddress();
//    printf("BINGBONG fp %p\n", fp);
//    printf("BINGBONG fp result %i\n", fp(10));
//  }
//}

#include <iostream>

#include <llvm-c/Target.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/Interpreter/Interpreter.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/PreprocessorOptions.h>

static void LLVMErrorHandler(void *UserData, char const *Message, bool GenCrashDiag)
{
  auto &Diags = *static_cast<clang::DiagnosticsEngine *>(UserData);
  Diags.Report(clang::diag::err_expected) << Message;
  exit(GenCrashDiag ? 70 : 1);
}

int main()
{
  using namespace llvm;
  using namespace llvm::orc;
  using namespace clang;

  LLVMInitializeNativeTarget();

  std::vector<char const *> ClangArgs = { "-Xclang", "-emit-llvm-only" };
  auto CI = cantFail(clang::IncrementalCompilerBuilder::create(ClangArgs));

  llvm::install_fatal_error_handler(LLVMErrorHandler, static_cast<void *>(&CI->getDiagnostics()));

  auto interpreter = Interpreter::create(std::move(CI));

  std::string line;
  std::cout << "> ";
  std::cout.flush();
  while(std::getline(std::cin, line))
  {
    auto res = interpreter.get()->ParseAndExecute(line);
    if(!res.success())
    {
    }

    std::cout << "> ";
    std::cout.flush();
  }
}
