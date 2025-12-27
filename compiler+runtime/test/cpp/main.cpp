#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <llvm-c/Target.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include <Interpreter/Compatibility.h>
#include <clang/Interpreter/CppInterOp.h>

#include <jank/c_api.h>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/error/report.hpp>
#include <clojure/core_native.hpp>

#ifdef JANK_PHASE_2
extern "C" void jank_load_clojure_core();
#endif

/* NOLINTNEXTLINE(bugprone-exception-escape): println can throw. */
int main(int const argc, char const **argv)
try
{
  return jank_init(argc, argv, /*init_default_ctx=*/true, [](int const argc, char const **argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    context.setOption("no-breaks", true);

    jank_load_clojure_core_native();

#ifdef JANK_PHASE_2
    jank_load_clojure_core();
    jank::runtime::__rt_ctx->module_loader.set_is_loaded("/clojure.core");
#else
    jank::runtime::__rt_ctx->load_module("/clojure.core", jank::runtime::module::origin::latest)
      .expect_ok();
#endif

    auto const res(context.run());
    if(context.shouldExit())
    {
      return res;
    }

    return res;
  });
}
/* Most exceptions are being caught in `jank_init`.
 * This piece here catches rest of them. */
catch(...)
{
  jank::util::println("Unknown exception thrown");
  return 1;
}
