#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

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
  { jank::common::translate("translate/do/expression/pass_integer.jank"); }

  template <> template <>
  void jank::expression_group::test<1>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/do/expression/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::expression_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/do/expression/fail_no_return.jank"); });
  }

  template <> template <>
  void jank::expression_group::test<3>()
  { jank::common::translate("translate/do/expression/pass_null.jank"); }
}
