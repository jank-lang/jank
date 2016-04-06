#include "prelude.hpp"

namespace jank_gen
{
/* This is the generated source. */
#include "jank-generated.hpp"
}

int main(int const argc, char ** const argv)
try
{
  jank_gen::_gen_poundmain();
}
catch(std::exception const &e)
{ std::cout << "exception: " << e.what() << std::endl; }
catch(...)
{ std::cout << "unknown exception"; }
