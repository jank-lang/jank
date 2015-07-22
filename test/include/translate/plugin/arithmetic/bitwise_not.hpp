#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct bitwise_not_test
  { };
  using bitwise_not_group = jest::group<bitwise_not_test>;
  static bitwise_not_group const bitwise_not_obj{ "arithmetic ~" };
}

namespace jest
{
  template <> template <>
  void jank::bitwise_not_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/bitwise_not/pass_integer.jank"); }
}
