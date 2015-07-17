#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  struct return_test{ };
  using return_group = jest::group<return_test>;
  static return_group const return_obj{ "non-generic function return" };
}

namespace jest
{
  template <> template <>
  void jank::return_group::test<0>()
  { jank::common::translate("translate/function/non-generic/return/pass_null_no_return.jank"); }

  template <> template <>
  void jank::return_group::test<1>()
  { jank::common::translate("translate/function/non-generic/return/pass_null_empty_return.jank"); }

  template <> template <>
  void jank::return_group::test<2>()
  { jank::common::translate("translate/function/non-generic/return/pass_single_value.jank"); }

  template <> template <>
  void jank::return_group::test<3>()
  { jank::common::translate("translate/function/non-generic/return/pass_single_value_param.jank"); }

  template <> template <>
  void jank::return_group::test<4>()
  { jank::common::translate("translate/function/non-generic/return/pass_single_value_implicit.jank"); }

  template <> template <>
  void jank::return_group::test<5>()
  { jank::common::translate("translate/function/non-generic/return/pass_single_value_implicit_param.jank"); }

  template <> template <>
  void jank::return_group::test<6>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_single_value_wrong_type.jank"); });
  }

  template <> template <>
  void jank::return_group::test<7>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_null_wrong_type.jank"); });
  }

  template <> template <>
  void jank::return_group::test<8>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_unknown_type.jank"); });
  }

  template <> template <>
  void jank::return_group::test<9>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_single_value_no_return.jank"); });
  }

  template <> template <>
  void jank::return_group::test<10>()
  {
    expect_exception<jank::translate::expect::error::internal::unimplemented>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_multiple_values.jank"); });
  }

  template <> template <>
  void jank::return_group::test<11>()
  {
    expect_exception<jank::translate::expect::error::internal::unimplemented>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_single_value_multiple_value_return.jank"); });
  }

  template <> template <>
  void jank::return_group::test<12>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_single_value_unkown_value.jank"); });
  }

  template <> template <>
  void jank::return_group::test<13>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_single_value_wrong_param_type.jank"); });
  }

  template <> template <>
  void jank::return_group::test<14>()
  {
    expect_exception<jank::translate::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_code_after_return.jank"); });
  }

  template <> template <>
  void jank::return_group::test<15>()
  {
    expect_exception<jank::translate::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_code_after_return_implicit.jank"); });
  }

  template <> template <>
  void jank::return_group::test<16>()
  { jank::common::translate("translate/function/non-generic/return/pass_implicit_function.jank"); }

  template <> template <>
  void jank::return_group::test<17>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/return/fail_implicit_function_wrong_type.jank"); });
  }
}
