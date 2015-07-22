#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct bitwise_xor_test
  { };
  using bitwise_xor_group = jest::group<bitwise_xor_test>;
  static bitwise_xor_group const bitwise_xor_obj{ "arithmetic ^" };
}

namespace jest
{
  template <> template <>
  void jank::bitwise_xor_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/bitwise_xor/pass_integer.jank"); }
}
