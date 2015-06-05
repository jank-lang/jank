#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct define_test{ };
  using define_group = jest::group<define_test>;
  static define_group const define_obj{ "non-generic variable define" };
}

namespace jest
{
  template <> template <>
  void jank::define_group::test<0>()
  { jank::common::translate("translate/variable/non-generic/define/pass_builtin_literal.jank"); }

  template <> template <>
  void jank::define_group::test<1>()
  { jank::common::translate("translate/variable/non-generic/define/pass_builtin_identifier.jank"); }

  template <> template <>
  void jank::define_group::test<2>()
  { jank::common::translate("translate/variable/non-generic/define/pass_builtin_parameter.jank"); }

  template <> template <>
  void jank::define_group::test<3>()
  { jank::common::translate("translate/variable/non-generic/define/pass_builtin_function.jank"); }

  template <> template <>
  void jank::define_group::test<4>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_unknown_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<5>()
  {
    expect_exception<jank::translate::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_missing_value.jank"); });
  }

  template <> template <>
  void jank::define_group::test<6>()
  {
    expect_exception<jank::translate::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_missing_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<7>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_incompatible_value.jank"); });
  }

  template <> template <>
  void jank::define_group::test<8>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_unknown_value_identifier.jank"); });
  }

  template <> template <>
  void jank::define_group::test<9>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_identifier_with_incompatible_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<10>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/variable/non-generic/define/fail_function_with_incompatible_type.jank"); });
  }
}
