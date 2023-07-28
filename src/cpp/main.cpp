#include <iostream>
#include <filesystem>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>

#include <jank/runtime/detail/object_util.hpp>

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

  GC_set_all_interior_pointers(1);
  GC_enable();
  /* TODO: This crashes now, with LLVM13. Looks like it's cleaning up things it shouldn't. */
  //GC_enable_incremental();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  char const *file{ argv[1] };

  jank::runtime::context rt_ctx;
  jank::jit::processor jit_prc;

  try
  {
    rt_ctx.eval_prelude(jit_prc);

    /* TODO: This doesn't handle macros properly, I think. */
    //{
    //  auto const mfile(jank::util::map_file(file));
    //  jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
    //  jank::read::parse::processor p_prc{ rt_ctx, l_prc.begin(), l_prc.end() };
    //  jank::analyze::processor an_prc{ rt_ctx };
    //  jank::codegen::processor cg_prc
    //  {
    //    rt_ctx,
    //    an_prc.analyze(p_prc.begin(), p_prc.end()).expect_ok_move()
    //  };
    //  std::cout << cg_prc.declaration_str() << std::endl;
    //  return 0;
    //}

    std::cout << jank::runtime::detail::to_string(rt_ctx.eval_file(file, jit_prc)) << std::endl;
  }
  catch(std::exception const &e)
  { fmt::print("Exception: {}", e.what()); }
  catch(jank::runtime::object_ptr const o)
  { fmt::print("Exception: {}", jank::runtime::detail::to_string(o)); }
  catch(jank::native_string const &s)
  { fmt::print("Exception: {}", s); }
  catch(jank::read::error const &e)
  { fmt::print("Read error: {}", e.message); }

  //std::string line;
  //std::cout << "> " << std::flush;
  //while(std::getline(std::cin, line))
  //{
  //  jit_prc.eval_string(line);
  //  std::cout << "> " << std::flush;
  //}
}
