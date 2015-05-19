#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct nest_test
  { };
  using nest_group = jest::group<nest_test>;
  static nest_group const nest_obj{ "non-generic function nest" };
}

namespace jest
{
  template <> template <>
  void jank::nest_group::test<0>()
  { jank::common::translate("translate/function/non-generic/nest/pass_define.jank"); }

  template <> template <>
  void jank::nest_group::test<1>()
  { jank::common::translate("translate/function/non-generic/nest/pass_redefine_outer.jank"); }

  template <> template <>
  void jank::nest_group::test<2>()
  { jank::common::translate("translate/function/non-generic/nest/pass_overload_outer.jank"); }

  template <> template <>
  void jank::nest_group::test<3>()
  { jank::common::translate("translate/function/non-generic/nest/pass_overload_self.jank"); }

  template <> template <>
  void jank::nest_group::test<4>()
  { jank::common::translate("translate/function/non-generic/nest/pass_overload_inner.jank"); }

  template <> template <>
  void jank::nest_group::test<5>()
  { jank::common::translate("translate/function/non-generic/nest/pass_capture_params.jank"); }

  template <> template <>
  void jank::nest_group::test<6>()
  { jank::common::translate("translate/function/non-generic/nest/pass_redefine_self.jank"); }

  template <> template <>
  void jank::nest_group::test<7>()
  {
    expect_exception<jank::translate::expect::error::type::overload>
    ([]{ jank::common::translate("translate/function/non-generic/nest/fail_multiple_inner_definition.jank"); });
  }

  template <> template <>
  void jank::nest_group::test<8>()
  { jank::common::translate("translate/function/non-generic/nest/pass_overload_outer_call_outer.jank"); }

}
