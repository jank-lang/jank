#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct bitwise_right_shift_test
  { };
  using bitwise_right_shift_group = jest::group<bitwise_right_shift_test>;
  static bitwise_right_shift_group const bitwise_right_shift_obj{ "arithmetic >>" };
}

namespace jest
{
  template <> template <>
  void jank::bitwise_right_shift_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/bitwise_right_shift/pass_integer.jank"); }
}
