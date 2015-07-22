#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct bitwise_and_test
  { };
  using bitwise_and_group = jest::group<bitwise_and_test>;
  static bitwise_and_group const bitwise_and_obj{ "arithmetic &" };
}

namespace jest
{
  template <> template <>
  void jank::bitwise_and_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/bitwise_and/pass_integer.jank"); }
}
