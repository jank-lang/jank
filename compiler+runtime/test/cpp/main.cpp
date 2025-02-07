#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <gc/gc.h>
#include <gc/gc_cpp.h>

#include <fmt/format.h>

#include <llvm-c/Target.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <clojure/core_native.hpp>

/* NOLINTNEXTLINE(bugprone-exception-escape): println can throw. */
int main(int const argc, char const **argv)
try
{
  std::locale::global(std::locale(""));

  GC_set_all_interior_pointers(1);
  GC_enable();

  llvm::llvm_shutdown_obj const Y{};

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();

  doctest::Context context;
  context.applyCommandLine(argc, argv);
  context.setOption("no-breaks", true);

  jank::runtime::__rt_ctx = new(GC) jank::runtime::context{};
  jank_load_clojure_core_native();
  /* TODO: Load latest here.
   * We're loading from source always due to a bug in how we generate symbols which is
   * leading to duplicate symbols being generated. */
  jank::runtime::__rt_ctx->load_module("/clojure.core", jank::runtime::module::origin::source)
    .expect_ok();

  auto const res(context.run());
  if(context.shouldExit())
  {
    return res;
  }

  return res;
}
/* TODO: Unify error handling. JEEZE! */
catch(std::exception const &e)
{
  fmt::println("Exception: {}", e.what());
}
catch(jank::runtime::object_ptr const o)
{
  fmt::println("Exception: {}", jank::runtime::to_string(o));
}
catch(jank::native_persistent_string const &s)
{
  fmt::println("Exception: {}", s);
}
catch(jank::read::error const &e)
{
  fmt::println("Read error ({} - {}): {}", e.start, e.end, e.message);
}
catch(...)
{
  fmt::println("Unknown exception thrown");
}
