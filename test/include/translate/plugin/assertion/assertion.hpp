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
  void jank::assertion_group::test<1>()
  { jank::common::interpret("translate/plugin/assertion/pass_true_message.jank"); }

  template <> template <>
  void jank::assertion_group::test<2>()
  { jank::common::interpret("translate/plugin/assertion/pass_not_true.jank"); }

  template <> template <>
  void jank::assertion_group::test<3>()
  { jank::common::interpret("translate/plugin/assertion/pass_not_true_message.jank"); }

  template <> template <>
  void jank::assertion_group::test<4>()
  {
    expect_exception<jank::interpret::expect::error::assertion::exception<>>
    ([]{ jank::common::interpret("translate/plugin/assertion/fail_false.jank"); });
  }

  template <> template <>
  void jank::assertion_group::test<5>()
  {
    expect_exception<jank::interpret::expect::error::assertion::exception<>>
    ([]{ jank::common::interpret("translate/plugin/assertion/fail_false_message.jank"); });
  }

  template <> template <>
  void jank::assertion_group::test<6>()
  {
    expect_exception<jank::interpret::expect::error::assertion::exception<>>
    ([]{ jank::common::interpret("translate/plugin/assertion/fail_not_false.jank"); });
  }

  template <> template <>
  void jank::assertion_group::test<7>()
  {
    expect_exception<jank::interpret::expect::error::assertion::exception<>>
    ([]{ jank::common::interpret("translate/plugin/assertion/fail_not_false_message.jank"); });
  }
}
