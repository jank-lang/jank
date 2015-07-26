#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct define_test{ };
  using define_group = jest::group<define_test>;
  static define_group const define_obj{ "do define" };
}

namespace jest
{
  template <> template <>
  void jank::define_group::test<0>()
  { jank::common::translate("translate/do/define/pass_shadow_variable.jank"); }

  template <> template <>
  void jank::define_group::test<1>()
  { jank::common::translate("translate/do/define/pass_shadow_function.jank"); }

  template <> template <>
  void jank::define_group::test<2>()
  { jank::common::translate("translate/do/define/pass_return.jank"); }

  template <> template <>
  void jank::define_group::test<3>()
  { jank::common::translate("translate/do/define/pass_within_if.jank"); }

  template <> template <>
  void jank::define_group::test<4>()
  { jank::common::translate("translate/do/define/pass_nested_return.jank"); }

  template <> template <>
  void jank::define_group::test<5>()
  {
    expect_exception<jank::translate::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("translate/do/define/fail_empty.jank"); });
  }
}
