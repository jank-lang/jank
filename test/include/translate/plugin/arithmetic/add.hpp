#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct add_test
  { };
  using add_group = jest::group<add_test>;
  static add_group const add_obj{ "arithmetic +" };
}

namespace jest
{
  template <> template <>
  void jank::add_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/add/pass_integer.jank"); }

  template <> template <>
  void jank::add_group::test<1>()
  { jank::common::interpret("translate/plugin/arithmetic/add/pass_real.jank"); }

  template <> template <>
  void jank::add_group::test<2>()
  { jank::common::interpret("translate/plugin/arithmetic/add/pass_string.jank"); }
}
