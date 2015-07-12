#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct call_test
  { };
  using call_group = jest::group<call_test>;
  static call_group const call_obj{ "assertions" };
}

namespace jest
{
  template <> template <>
  void jank::call_group::test<0>()
  { jank::common::interpret("translate/plugin/assertion/pass_true.jank"); }

  template <> template <>
  void jank::call_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::assertion::exception<>>
    ([]{ jank::common::interpret("translate/plugin/assertion/fail_false.jank"); });
  }
}
