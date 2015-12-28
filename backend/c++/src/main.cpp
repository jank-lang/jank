#include <iostream>
#include <string>
#include <sstream>

namespace jank
{
  /* -- Primitives -- */
  using integer = int64_t;
  using string = std::string;

  /* -- Output -- */
  template <typename T>
  string print(T const &t)
  {
    std::stringstream ss;
    ss << t;
    std::cout << t.rdbuf() << std::endl;
    return ss.str();
  }

  /* -- Input -- */
  string input()
  {
    string s;
    std::getline(std::cin, s);
    return s;
  }

  /* -- Primitive arithmetic -- */
  template <typename A, typename B>
  auto _gen_plus_(A const &a, B const &b)
  { return a + b; }

  template <typename A, typename B>
  auto _gen_minus_(A const &a, B const &b)
  { return a - b; }

  template <typename A, typename B>
  auto _gen_asterisk_(A const &a, B const &b)
  { return a * b; }

  template <typename A, typename B>
  auto _gen_slash_(A const &a, B const &b)
  { return a / b; }

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
