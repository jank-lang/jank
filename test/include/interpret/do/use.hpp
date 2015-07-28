#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct use_test{ };
  using use_group = jest::group<use_test>;
  static use_group const use_obj{ "do usage" };
}

namespace jest
{
  template <> template <>
  void jank::use_group::test<0>()
  { jank::common::translate("interpret/do/use/pass_return.jank"); }

  template <> template <>
  void jank::use_group::test<1>()
  { jank::common::translate("interpret/do/use/pass_nested_return.jank"); }

  template <> template <>
  void jank::use_group::test<2>()
  { jank::common::translate("interpret/do/use/pass_shadow_binding.jank"); }

  template <> template <>
  void jank::use_group::test<3>()
  { jank::common::translate("interpret/do/use/pass_shadow_function.jank"); }

  template <> template <>
  void jank::use_group::test<4>()
  { jank::common::translate("interpret/do/use/pass_within_if.jank"); }
}
