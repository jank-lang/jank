#pragma once

#include <prelude/object.hpp>

namespace jank
{
  inline object print(object const &o)
  {
    o.visit
    (
      [](auto &&data)
      { std::cout << data; }
    );
    return JANK_NIL;
  }

  inline object println(object const &o)
  {
    print(o);
    std::cout << std::endl;
    return JANK_NIL;
  }

  inline object read_gen_minus_line()
  {
    string input;
    std::getline(std::cin, input);
    return object{ std::move(input) };
  }
}
