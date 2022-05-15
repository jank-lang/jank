#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  inline object_ptr print(object_ptr const &o)
  {
    std::cout << *o;
    return JANK_NIL;
  }

  inline object_ptr println(object_ptr const &o)
  {
    print(o);
    std::cout << "\n";
    return JANK_NIL;
  }

  inline object_ptr flush()
  {
    std::cout << std::flush;
    return JANK_NIL;
  }

  inline object_ptr read_gen_minus_line()
  {
    /* TODO: Optimize by reading into string_type. */
    std::string input;
    std::getline(std::cin, input);
    return make_box<string>(std::move(input));
  }
}
