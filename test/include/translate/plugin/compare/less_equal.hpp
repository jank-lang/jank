#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct less_equal_test
  { };
  using less_equal_group = jest::group<less_equal_test>;
  static less_equal_group const less_equal_obj{ "comparison <=" };
}

namespace jest
{
  template <> template <>
  void jank::less_equal_group::test<0>()
  { jank::common::interpret("translate/plugin/compare/less_equal/pass_integer.jank"); }

  template <> template <>
  void jank::less_equal_group::test<1>()
  { jank::common::interpret("translate/plugin/compare/less_equal/pass_real.jank"); }

  template <> template <>
  void jank::less_equal_group::test<2>()
  { jank::common::interpret("translate/plugin/compare/less_equal/pass_string.jank"); }
}
