#include <iostream>
#include <vector>
#include <filesystem>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>
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
  //auto start = std::chrono::high_resolution_clock::now();
  jank::jit::processor jit_prc;
  //auto end = std::chrono::high_resolution_clock::now();
  //std::chrono::duration<double> diff = end - start;
  //std::cout << "jit startup time: " << diff.count() << "s" << std::endl;

  rt_ctx.eval_prelude(an_ctx, jit_prc);

  auto const mfile(jank::util::map_file(file));
  jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
  jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
  jank::analyze::processor an_prc
  { rt_ctx, p_prc.begin(), p_prc.end() };
  jank::codegen::processor cg_prc
  {
    rt_ctx,
    an_ctx,
    an_prc.result(an_ctx).expect_ok_move().unwrap()
  };
  std::cout << cg_prc.declaration_str() << std::endl;
  std::cout << cg_prc.expression_str() << std::endl;

  std::cout << jit_prc.eval(rt_ctx, cg_prc).expect_ok().unwrap()->to_string() << std::endl;

  //std::string line;
  //std::cout << "> " << std::flush;
  //while(std::getline(std::cin, line))
  //{
  //  jit_prc.eval_string(line);
  //  std::cout << "> " << std::flush;
  //}
}
