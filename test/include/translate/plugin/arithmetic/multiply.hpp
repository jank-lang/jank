#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct multiply_test
  { };
  using multiply_group = jest::group<multiply_test>;
  static multiply_group const multiply_obj{ "arithmetic *" };
}

namespace jest
{
  template <> template <>
  void jank::multiply_group::test<0>()
  { jank::common::interpret("translate/plugin/arithmetic/multiply/pass_integer.jank"); }

  template <> template <>
  void jank::multiply_group::test<1>()
  { jank::common::interpret("translate/plugin/arithmetic/multiply/pass_real.jank"); }

  template <> template <>
  void jank::multiply_group::test<2>()
  { jank::common::interpret("translate/plugin/arithmetic/multiply/pass_string.jank"); }

  template <> template <>
  void jank::multiply_group::test<3>()
  {
    expect_exception<jank::interpret::expect::error::type::exception<>>
    ([]{ jank::common::interpret("translate/plugin/arithmetic/multiply/fail_negative_string.jank"); });
  }
}
