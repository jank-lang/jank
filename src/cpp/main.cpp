#include <iostream>
#include <vector>
#include <filesystem>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#pragma clang diagnostic pop

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/evaluate/context.hpp>
#include <jank/jit/processor.hpp>

int main(int const argc, char const **argv)
{
  if(argc < 2)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  char const *file{ argv[1] };

  jank::runtime::context rt_ctx;
  jank::analyze::context an_ctx{ rt_ctx };
  jank::evaluate::context eval_ctx{ rt_ctx };

  rt_ctx.eval_prelude(an_ctx);

  auto const mfile(jank::util::map_file(file));
  jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
  jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
  jank::analyze::processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };
  jank::codegen::processor cg_prc{ rt_ctx, an_ctx, an_prc.begin(an_ctx), an_prc.end(an_ctx), an_prc.total_forms };
  //std::cout << cg_prc.str() << std::endl;

  jank::jit::processor jit_prc{ rt_ctx };
  jit_prc.eval(cg_prc);
}
