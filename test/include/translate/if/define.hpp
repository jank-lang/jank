#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct define_test{ };
  using define_group = jest::group<define_test>;
  static define_group const define_obj{ "if define" };
}

namespace jest
{
  template <> template <>
  void jank::define_group::test<0>()
  { jank::common::translate("translate/if/define/pass_empty.jank"); }

  // TODO
  // without else
  // with else
  // without both
  // with null condition
  // with integer condition
  // with bool condition
  // with bool function condition
  // with return
  // with return and else return
}
