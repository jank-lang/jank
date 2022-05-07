#include <iostream>
#include <vector>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>

int main(int const argc, char const **argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }
  char const *file{ argv[1] };

  auto const mfile(jank::util::map_file(file));
  auto lexer(jank::read::lex::processor{ { mfile->head, mfile->size } });
  (void)lexer;
}
