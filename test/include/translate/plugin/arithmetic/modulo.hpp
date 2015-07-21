#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct modulo_test
  { };
  using modulo_group = jest::group<modulo_test>;
  static modulo_group const modulo_obj{ "arithmetic %" };
}

namespace jest
{
  template <> template <>
  void jank::modulo_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/modulo/pass_integer.jank"); }
}
