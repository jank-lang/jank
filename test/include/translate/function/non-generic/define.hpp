#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct define_test{ };
  using define_group = jest::group<define_test>;
  static define_group const define_obj{ "non-generic function define" };
}

namespace jest
{
  template <> template <>
  void jank::define_group::test<0>()
  { jank::common::translate("translate/function/non-generic/define/pass_empty.jank"); }

  template <> template <>
  void jank::define_group::test<1>()
  { jank::common::translate("translate/function/non-generic/define/pass_primitive.jank"); }

  template <> template <>
  void jank::define_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::syntax::syntax<>>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_missing_param_name.jank"); });
  }

  template <> template <>
  void jank::define_group::test<3>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<4>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_multiple_definition.jank"); });
  }

  template <> template <>
  void jank::define_group::test<5>()
  { jank::common::translate("translate/function/non-generic/define/pass_return_primitive.jank"); }

  template <> template <>
  void jank::define_group::test<6>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_invalid_return_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<7>()
  {
    expect_exception<jank::translate::expect::error::syntax::syntax<>>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_no_param_list.jank"); });
  }

  template <> template <>
  void jank::define_group::test<8>()
  {
    expect_exception<jank::translate::expect::error::syntax::syntax<>>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_no_return_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<9>()
  { jank::common::translate("translate/function/non-generic/define/pass_return_single_value.jank"); }

  template <> template <>
  void jank::define_group::test<10>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/define/fail_return_incorrect_type.jank"); });
  }

  template <> template <>
  void jank::define_group::test<11>()
  { jank::common::translate("translate/function/non-generic/define/pass_body.jank"); }
}
