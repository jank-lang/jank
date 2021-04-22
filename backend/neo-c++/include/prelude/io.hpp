#pragma once

#include <prelude/object.hpp>

namespace jank
{
  inline object print(object const &o)
  {
    o.visit
    (
      [](auto const &data)
      { std::cout << data; }
    );
    return JANK_NIL;
  }

  inline object println(object const &o)
  {
    print(o);
    std::cout << "\n";
    return JANK_NIL;
  }

  inline object flush()
  {
    std::cout << std::flush;
    return JANK_NIL;
  }

  inline object read_gen_minus_line()
  {
    detail::string input;
    std::getline(std::cin, input);
    return object{ std::move(input) };
  }
}
