#include <iostream>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#pragma clang diagnostic pop

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/evaluate/context.hpp>

int main(int const argc, char const **argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }
  char const *file{ argv[1] };

  //jank::runtime::context rt_ctx;
  //jank::analyze::processor anal_prc{ rt_ctx };
  //jank::evaluate::context eval_ctx{ rt_ctx };

  auto const mfile(jank::util::map_file(file));
  std::cout << "file: \n" << mfile.expect_ok().head << std::endl;
  //jank::read::lex::processor l_prc{ { mfile.unwrap().head, mfile.unwrap().size } };
  //jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
  //std::cout << eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok()))->to_string() << std::endl;

  cling::Interpreter interp(argc, argv, LLVM_PATH);
}
