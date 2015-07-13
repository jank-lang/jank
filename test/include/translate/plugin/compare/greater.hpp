#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct greater_test
  { };
  using greater_group = jest::group<greater_test>;
  static greater_group const greater_obj{ "comparison >" };
}

namespace jest
{
  template <> template <>
  void jank::greater_group::test<0>()
  { jank::common::interpret("translate/plugin/compare/greater/pass_integer.jank"); }

  template <> template <>
  void jank::greater_group::test<1>()
  { jank::common::interpret("translate/plugin/compare/greater/pass_real.jank"); }

  template <> template <>
  void jank::greater_group::test<2>()
  { jank::common::interpret("translate/plugin/compare/greater/pass_string.jank"); }
}
