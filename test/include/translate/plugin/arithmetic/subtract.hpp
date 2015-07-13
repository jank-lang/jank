#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct subtract_test
  { };
  using subtract_group = jest::group<subtract_test>;
  static subtract_group const subtract_obj{ "arithmetic -" };
}

namespace jest
{
  template <> template <>
  void jank::subtract_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/subtract/pass_integer.jank"); }

  template <> template <>
  void jank::subtract_group::test<1>()
  { jank::common::interpret("translate/plugin/arithmetic/subtract/pass_real.jank"); }
}
