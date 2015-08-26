#pragma once

#include <jest/jest.hpp>

#include <jank/translate/expect/error/type/overload.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct overload_test
  { };
  using overload_group = jest::group<overload_test>;
  static overload_group const overload_obj{ "non-generic function overload" };
}

namespace jest
{
  template <> template <>
  void jank::overload_group::test<0>()
  { jank::common::translate("translate/function/non-generic/overload/pass_different_param_count.jank"); }

  template <> template <>
  void jank::overload_group::test<1>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/overload/fail_multiple_definition.jank"); });
  }

  template <> template <>
  void jank::overload_group::test<2>()
  { jank::common::translate("translate/function/non-generic/overload/pass_same_param_count.jank"); }

  template <> template <>
  void jank::overload_group::test<3>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/overload/fail_return_type.jank"); });
  }
}
