#include <llvm-c/Target.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include <clojure/core_native.hpp>
#include <clojure/string_native.hpp>

#include <jank/compiler_native.hpp>
#include <jank/perf_native.hpp>
#include <jank/init.hpp>
#include <jank/profile/time.hpp>
#include <jank/runtime/context.hpp>
#include <jank/util/try.hpp>

namespace jank
{

  int init(int const argc,
           char const **argv,
           bool init_default_ctx,
           void (*fn)(int const, char const **))
  {
    JANK_TRY
    {
      /* To handle UTF-8 Text , we set the locale to the current environment locale
     * Usage of the local locale allows better localization.
     * Notably this might make text encoding become more platform dependent.
     */
      std::locale::global(std::locale(""));

      /* The GC needs to enabled even before arg parsing, since our native types,
     * like strings, use the GC for allocations. It can still be configured later. */
      GC_set_all_interior_pointers(1);
      GC_enable();

      //obj::symbol_ref r;
      //r = make_box<obj::symbol>("foo");
      //if(r)
      //{
      //  object_ref o;
      //  o = erase(r);
      //  util::println("r {}", r->to_code_string());
      //}

      //return 0;

      llvm::llvm_shutdown_obj const Y{};

      llvm::InitializeNativeTarget();
      llvm::InitializeNativeTargetAsmParser();
      llvm::InitializeNativeTargetAsmPrinter();

#ifdef JANK_INCREMENTAL_GC
      GC_enable_incremental();
#endif

      if(init_default_ctx)
      {
        runtime::__rt_ctx = new(GC) runtime::context{};
      }

      fn(argc, argv);
    }
    JANK_CATCH_THEN(jank::util::print_exception, return 1)

    return 0;
  }
}
