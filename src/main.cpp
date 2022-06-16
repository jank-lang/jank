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

int main(int const argc, char const **argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }
  char const *file{ argv[1] };

  auto const mfile(jank::util::map_file(file));
  auto lexer(jank::read::lex::processor{ { mfile.unwrap().head, mfile.unwrap().size } });
  (void)lexer;

  jank::runtime::context ctx;

  cling::Interpreter interp(argc, argv, LLVM_PATH);
}
