#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct bitwise_left_shift_test
  { };
  using bitwise_left_shift_group = jest::group<bitwise_left_shift_test>;
  static bitwise_left_shift_group const bitwise_left_shift_obj{ "arithmetic <<" };
}

namespace jest
{
  template <> template <>
  void jank::bitwise_left_shift_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/bitwise_left_shift/pass_integer.jank"); }
}
