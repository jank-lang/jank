#include <llvm-c/Target.h>
#include <llvm/ExecutionEngine/Orc/AbsoluteSymbols.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/Mangling.h>

#include <jank/c_api.h>
#include <jank/util/try.hpp>
#include <jank/runtime/context.hpp>
#include <jank/aot/resource.hpp>

extern "C"
{
  int jank_init_dynamic(int const argc,
                        char const ** const argv,
                        jank_bool const init_default_ctx,
                        char const * const pch_data,
                        jank_usize const pch_size,
                        int (*fn)(int const, char const ** const))
  {
    jank_init_base();

    llvm::llvm_shutdown_obj const Y{};

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    /* This try needs to come AFTER we initialize LLVM. If we have it before, we'll be
     * unable to catch some exceptions thrown by JIT-compiled frames. */
    JANK_TRY
    {
      if(pch_data)
      {
        jank::aot::register_resource("incremental.pch", { pch_data, pch_size });
      }
      if(init_default_ctx)
      {
        jank::runtime::__rt_ctx = new(UseGC) jank::runtime::context{};
      }

      return fn(argc, argv);
    }
    JANK_CATCH_THEN(jank::util::print_exception, return 1)

    return 0;
  }
}
