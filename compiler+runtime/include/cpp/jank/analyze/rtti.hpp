#pragma once

#include <llvm/Support/Casting.h>

#include <jank/analyze/expression.hpp>

namespace llvm
{
  template <typename To>
  requires jank::analyze::expression_like<To>
  struct isa_impl<To, jank::analyze::expression>
  {
    static bool doit(jank::analyze::expression const &val)
    {
      return val.kind == To::expr_kind;
    }
  };
}
