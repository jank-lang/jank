#pragma once

#include <prelude/object.hpp>

namespace jank
{
  inline object_ptr print(object_ptr const &o)
  {
    std::cout << o->to_string();
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
    detail::string_type input;
    std::getline(std::cin, input);
    return make_box<string>(std::move(input));
  }
}
