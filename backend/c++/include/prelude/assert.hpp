#pragma once

#include "primitive.hpp"

namespace jank
{
  void assert_bang(boolean const b, string const &s)
  {
    if(!b)
    { throw std::runtime_error{ "(assertion failure) " + s }; }
  }
  void assert_bang(boolean const b)
  { assert_bang(b, ""); }
  void assert_not_bang(boolean const b)
  { assert_bang(!b); }
  void assert_not_bang(boolean const b, string const &s)
  { assert_bang(!b, s); }
  void assert_unreachable_bang()
  { assert_bang(false, "unreachable code reached"); }
}
