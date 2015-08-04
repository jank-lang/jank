#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct expression_test{ };
  using expression_group = jest::group<expression_test>;
  static expression_group const expression_obj{ "if expression" };
}

namespace jest
{
  template <> template <>
  void jank::expression_group::test<0>()
  { jank::common::translate("translate/if/expression/pass_integer.jank"); }

  template <> template <>
  void jank::expression_group::test<1>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/if/expression/fail_without_else.jank"); });
  }

  template <> template <>
  void jank::expression_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/if/expression/fail_different_types.jank"); });
  }

  template <> template <>
  void jank::expression_group::test<3>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/if/expression/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::expression_group::test<4>()
  { jank::common::translate("translate/if/expression/pass_null.jank"); }
}
