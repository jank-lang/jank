#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct bind_test{ };
  using bind_group = jest::group<bind_test>;
  static bind_group const bind_obj{ "non-generic lambda binding" };
}

namespace jest
{
  template <> template <>
  void jank::bind_group::test<0>()
  { jank::common::translate("translate/lambda/non-generic/bind/pass_simple.jank"); }

  template <> template <>
  void jank::bind_group::test<1>()
  { jank::common::translate("translate/lambda/non-generic/bind/pass_with_type.jank"); }

  template <> template <>
  void jank::bind_group::test<2>()
  { jank::common::translate("translate/lambda/non-generic/bind/pass_call.jank"); }

  template <> template <>
  void jank::bind_group::test<3>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/lambda/non-generic/bind/fail_incorrect_type.jank"); });
  }
}
