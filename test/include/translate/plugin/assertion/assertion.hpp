#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct assertion_test
  { };
  using assertion_group = jest::group<assertion_test>;
  static assertion_group const assertion_obj{ "assertions" };
}

namespace jest
{
  template <> template <>
  void jank::assertion_group::test<0>()
  { jank::common::interpret("translate/plugin/assertion/pass_true.jank"); }

  template <> template <>
  void jank::assertion_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::assertion::exception<>>
    ([]{ jank::common::interpret("translate/plugin/assertion/fail_false.jank"); });
  }
}
