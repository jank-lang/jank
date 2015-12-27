#include <iostream>
#include <string>

namespace jank
{
  using integer = int64_t;
  using string = std::string;

  template <typename T>
  string print(T const &t)
  {
    std::cout << t << std::endl;
    return ""; // TODO
  }

/* This is the generated source. */
#include "jank-generated.hpp"
}

int main(int const argc, char ** const argv)
try
{
  jank::_gen_pound_main();
}
catch(std::exception const &e)
{ std::cout << "exception: " << e.what() << std::endl; }
catch(...)
{ std::cout << "unknown exception"; }
