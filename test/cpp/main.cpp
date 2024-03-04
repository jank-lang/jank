#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <llvm-c/Target.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

int main(int const argc, char const **argv)
{
  GC_set_all_interior_pointers(1);
  GC_enable();

  llvm::llvm_shutdown_obj Y{};

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();

  doctest::Context context;
  context.applyCommandLine(argc, argv);
  context.setOption("no-breaks", true);

  auto const res(context.run());
  if(context.shouldExit())
  {
    return res;
  }

  return res;
}
