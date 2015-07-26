#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct use_test{ };
  using use_group = jest::group<use_test>;
  static use_group const use_obj{ "if usage" };
}

namespace jest
{
  template <> template <>
  void jank::use_group::test<0>()
  { jank::common::translate("interpret/if/use/pass_with_return.jank"); }

  template <> template <>
  void jank::use_group::test<1>()
  { jank::common::translate("interpret/if/use/pass_with_return_and_else.jank"); }
}
