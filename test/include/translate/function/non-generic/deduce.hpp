#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  struct deduce_test{ };
  using deduce_group = jest::group<deduce_test>;
  static deduce_group const deduce_obj{ "non-generic function return type deduction" };
}

namespace jest
{
  template <> template <>
  void jank::deduce_group::test<0>()
  { jank::common::translate("translate/function/non-generic/deduce/pass_simple.jank"); }

  template <> template <>
  void jank::deduce_group::test<1>()
  { jank::common::translate("translate/function/non-generic/deduce/pass_if.jank"); }

  template <> template <>
  void jank::deduce_group::test<2>()
  { jank::common::translate("translate/function/non-generic/deduce/pass_do.jank"); }

  template <> template <>
  void jank::deduce_group::test<3>()
  { jank::common::translate("translate/function/non-generic/deduce/pass_with_normal_return.jank"); }

  template <> template <>
  void jank::deduce_group::test<4>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/deduce/fail_mismatched_types.jank"); });
  }
}
