#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct less_test
  { };
  using less_group = jest::group<less_test>;
  static less_group const less_obj{ "comparison <" };
}

namespace jest
{
  template <> template <>
  void jank::less_group::test<0>()
  { jank::common::interpret("translate/plugin/compare/less/pass_integer.jank"); }

  template <> template <>
  void jank::less_group::test<1>()
  { jank::common::interpret("translate/plugin/compare/less/pass_real.jank"); }

  template <> template <>
  void jank::less_group::test<2>()
  { jank::common::interpret("translate/plugin/compare/less/pass_string.jank"); }
}
