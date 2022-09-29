#include <iostream>
#include <vector>
#include <filesystem>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#pragma clang diagnostic pop

#include <jank/util/mapped_file.hpp>
#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/context.hpp>
#include <jank/evaluate/context.hpp>

int main(int const argc, char const **argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }
  char const *file{ argv[1] };

  jank::runtime::context rt_ctx;
  jank::analyze::context an_ctx{ rt_ctx };
  jank::evaluate::context eval_ctx{ rt_ctx };

  rt_ctx.eval_prelude(an_ctx);

  auto const mfile(jank::util::map_file(file));
  jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
  jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
  jank::analyze::processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };
  jank::codegen::context cg_ctx{ rt_ctx, an_ctx, an_prc.begin(an_ctx), an_prc.end(an_ctx), an_prc.total_forms };
  //std::cout << cg_ctx.str() << std::endl;

  auto const jank_location(jank::util::process_location().unwrap().parent_path());
  auto const cling_args(jank::util::make_array(argv[0], "-std=c++17"));
  cling::Interpreter jit(cling_args.size(), cling_args.data(), LLVMDIR);

  jit.AddIncludePath(jank_location.string() + "/../include");
  jit.AddIncludePath(jank_location.string() + "/../include/cpp");
  /* TODO: Figure out how to make this easier for dev. */
  jit.AddIncludePath(jank_location.string() + "/vcpkg_installed/x64-linux/include");
  jit.AddIncludePath(jank_location.string() + "/vcpkg_installed/x64-osx/include");

  /* TODO: Pre-compiled prelude. */
  jit.process("#include \"jank/runtime/obj/number.hpp\"");
  jit.process("#include \"jank/runtime/obj/function.hpp\"");
  jit.process("#include \"jank/runtime/obj/map.hpp\"");
  jit.process("#include \"jank/runtime/obj/list.hpp\"");
  jit.process("#include \"jank/runtime/obj/vector.hpp\"");
  jit.process("#include \"jank/runtime/obj/set.hpp\"");
  jit.process("#include \"jank/runtime/obj/string.hpp\"");
  jit.process("#include \"jank/runtime/ns.hpp\"");
  jit.process("#include \"jank/runtime/var.hpp\"");
  jit.process("#include \"jank/runtime/context.hpp\"");
  jit.process(cg_ctx.str());
}
