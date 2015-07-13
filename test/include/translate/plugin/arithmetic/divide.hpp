#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct divide_test
  { };
  using divide_group = jest::group<divide_test>;
  static divide_group const divide_obj{ "arithmetic /" };
}

namespace jest
{
  template <> template <>
  void jank::divide_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/divide/pass_integer.jank"); }

  template <> template <>
  void jank::divide_group::test<1>()
  { jank::common::interpret("translate/plugin/arithmetic/divide/pass_real.jank"); }
}
