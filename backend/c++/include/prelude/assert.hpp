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

namespace jank_gen
{
  void assert_gen_bang1566167926(boolean const b, string const &s)
  { jank::assert_bang(b, s); }
  void assert_gen_bang_gen_minus1906603266(boolean const b)
  { jank::assert_bang(b, ""); }
  void assert_gen_minusnot_gen_bang_gen_minus1906603266(boolean const b)
  { jank::assert_not_bang(b); }
  void assert_gen_minusnot_gen_bang1566167926(boolean const b, string const &s)
  { jank::assert_not_bang(b, s); }
  void assert_gen_minusunreachable_gen_bang2018684456()
  { jank::assert_unreachable_bang(); }
}
