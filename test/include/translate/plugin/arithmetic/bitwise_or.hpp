#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct bitwise_or_test
  { };
  using bitwise_or_group = jest::group<bitwise_or_test>;
  static bitwise_or_group const bitwise_or_obj{ "arithmetic |" };
}

namespace jest
{
  template <> template <>
  void jank::bitwise_or_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/bitwise_or/pass_integer.jank"); }
}
