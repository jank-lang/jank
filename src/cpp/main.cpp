#include <iostream>
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

  GC_INIT();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  char const *file{ argv[1] };

  jank::runtime::context rt_ctx;
  jank::jit::processor jit_prc;

  rt_ctx.eval_prelude(jit_prc);

  //{
  //  auto const mfile(jank::util::map_file(file));
  //  jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
  //  jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
  //  jank::analyze::processor an_prc{ rt_ctx };
  //  jank::codegen::processor cg_prc
  //  {
  //    rt_ctx,
  //    an_prc.analyze(p_prc.begin(), p_prc.end()).expect_ok_move()
  //  };
  //  std::cout << cg_prc.declaration_str() << std::endl;
  //}

  try
  { std::cout << rt_ctx.eval_file(file, jit_prc)->to_string() << std::endl; }
  catch(std::exception const &e)
  { fmt::print("Exception: {}", e.what()); }
  catch(jank::runtime::object_ptr const o)
  { fmt::print("Exception: {}", o->to_string()); }

  //std::string line;
  //std::cout << "> " << std::flush;
  //while(std::getline(std::cin, line))
  //{
  //  jit_prc.eval_string(line);
  //  std::cout << "> " << std::flush;
  //}
}
