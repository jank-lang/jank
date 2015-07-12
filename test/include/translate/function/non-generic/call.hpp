#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct call_test
  { };
  using call_group = jest::group<call_test>;
  static call_group const call_obj{ "non-generic function call" };
}

namespace jest
{
  template <> template <>
  void jank::call_group::test<0>()
  { jank::common::translate("translate/function/non-generic/call/pass_empty.jank"); }

  template <> template <>
  void jank::call_group::test<1>()
  { jank::common::translate("translate/function/non-generic/call/pass_print.jank"); }

  template <> template <>
  void jank::call_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::lookup::exception<>>
    ([]{ jank::common::translate("translate/function/non-generic/call/fail_invalid_function.jank"); });
  }

  template <> template <>
  void jank::call_group::test<3>()
  { jank::common::translate("translate/function/non-generic/call/pass_print_primitive.jank"); }

  template <> template <>
  void jank::call_group::test<4>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/call/fail_too_few_params.jank"); });
  }

  template <> template <>
  void jank::call_group::test<5>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/call/fail_too_many_params.jank"); });
  }

  template <> template <>
  void jank::call_group::test<6>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/call/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::call_group::test<7>()
  { jank::common::translate("translate/function/non-generic/call/pass_chain.jank"); }

  template <> template <>
  void jank::call_group::test<8>()
  { jank::common::translate("translate/function/non-generic/call/pass_function_call_param.jank"); }

  template <> template <>
  void jank::call_group::test<9>()
  { jank::common::translate("translate/function/non-generic/call/pass_recursion.jank"); }
}
