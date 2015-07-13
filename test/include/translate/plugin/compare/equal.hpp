#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct equal_test
  { };
  using equal_group = jest::group<equal_test>;
  static equal_group const equal_obj{ "comparison ==" };
}

namespace jest
{
  template <> template <>
  void jank::equal_group::test<0>()
  { jank::common::interpret("translate/plugin/compare/equal/pass_integer.jank"); }

  template <> template <>
  void jank::equal_group::test<1>()
  { jank::common::interpret("translate/plugin/compare/equal/pass_real.jank"); }

  template <> template <>
  void jank::equal_group::test<2>()
  { jank::common::interpret("translate/plugin/compare/equal/pass_string.jank"); }
}
