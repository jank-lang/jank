#pragma once

#include "primitive.hpp"

namespace jank
{
  void assert_gen_bang(boolean const b, string const &s)
  {
    if(!b)
    { throw std::runtime_error{ "(assertion failure) " + s }; }
  }
  void assert_gen_bang(boolean const b)
  { assert_gen_bang(b, ""); }
  void assert_gen_minusnot_gen_bang(boolean const b)
  { assert_gen_bang(!b); }
  void assert_gen_minusnot_gen_bang(boolean const b, string const &s)
  { assert_gen_bang(!b, s); }
  void assert_gen_minusunreachable_gen_bang()
  { assert_gen_bang(false, "unreachable code reached"); }
}
