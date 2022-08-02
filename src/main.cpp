#include <iostream>
#include <vector>
#include <experimental/array>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
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

  jank::runtime::context rt_ctx;
  jank::analyze::context anal_ctx;
  jank::analyze::processor anal_prc{ rt_ctx };
  jank::evaluate::context eval_ctx{ rt_ctx };

  auto const mfile(jank::util::map_file(file));
  jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
  jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
  std::cout << eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx))->to_string() << std::endl;

  auto const cling_args(std::experimental::make_array(argv[0], "-std=c++17"));
  cling::Interpreter jit(cling_args.size(), cling_args.data(), LLVM_PATH);
  jit.AddIncludePath("/home/jeaye/projects/jank/lib/immer");
  jit.AddIncludePath("/home/jeaye/projects/jank/lib/magic_enum/include");
  jit.AddIncludePath("/home/jeaye/projects/jank/include");
  jit.process("#include <iostream>");
  jit.process("#include \"jank/runtime/object.hpp\"");
  jit.process("#include \"jank/runtime/obj/string.hpp\"");
  jit.process("std::cout << jank::runtime::obj::string(\"meow\").to_string() << std::endl;");
  jit.process("std::cout << jank::runtime::make_box<jank::runtime::obj::string>(\"meow\")->to_string() << std::endl;");
}
