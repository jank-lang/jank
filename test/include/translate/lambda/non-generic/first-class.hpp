#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct first_class_test{ };
  using first_class_group = jest::group<first_class_test>;
  static first_class_group const first_class_obj{ "non-generic lambda as first-class objects" };
}

namespace jest
{
  template <> template <>
  void jank::first_class_group::test<0>()
  { jank::common::translate("translate/lambda/non-generic/first-class/pass_simple.jank"); }

  template <> template <>
  void jank::first_class_group::test<1>()
  { jank::common::translate("translate/lambda/non-generic/first-class/pass_with_params.jank"); }

  template <> template <>
  void jank::first_class_group::test<2>()
  { jank::common::translate("translate/lambda/non-generic/first-class/pass_as_param.jank"); }

  template <> template <>
  void jank::first_class_group::test<3>()
  { jank::common::translate("translate/lambda/non-generic/first-class/pass_higher_order_lambda.jank"); }

  template <> template <>
  void jank::first_class_group::test<4>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/lambda/non-generic/first-class/fail_incorrect_return_type.jank"); });
  }

  template <> template <>
  void jank::first_class_group::test<5>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/lambda/non-generic/first-class/fail_incorrect_param_type.jank"); });
  }

  // recursion from lambda into HO function
}
