#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct expression_test{ };
  using expression_group = jest::group<expression_test>;
  static expression_group const expression_obj{ "do expression" };
}

namespace jest
{
  template <> template <>
  void jank::expression_group::test<0>()
  { jank::common::interpret("interpret/do/expression/pass_integer.jank"); }

  template <> template <>
  void jank::expression_group::test<1>()
  { jank::common::interpret("interpret/do/expression/pass_null.jank"); }
}
