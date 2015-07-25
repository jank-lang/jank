#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct define_test{ };
  using define_group = jest::group<define_test>;
  static define_group const define_obj{ "if define" };
}

namespace jest
{
  template <> template <>
  void jank::define_group::test<0>()
  { jank::common::translate("translate/if/define/pass_without_else.jank"); }

  template <> template <>
  void jank::define_group::test<1>()
  { jank::common::translate("translate/if/define/pass_with_else.jank"); }

  template <> template <>
  void jank::define_group::test<2>()
  {
    expect_exception<jank::translate::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("translate/if/define/fail_without_both.jank"); });
  }

  template <> template <>
  void jank::define_group::test<3>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/if/define/fail_null_condition.jank"); });
  }

  template <> template <>
  void jank::define_group::test<4>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/if/define/fail_integer_condition.jank"); });
  }

  template <> template <>
  void jank::define_group::test<5>()
  { jank::common::translate("translate/if/define/pass_with_return.jank"); }

  template <> template <>
  void jank::define_group::test<6>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("translate/if/define/fail_with_return_missing_outer.jank"); });
  }

  template <> template <>
  void jank::define_group::test<7>()
  { jank::common::translate("translate/if/define/pass_with_return_and_else.jank"); }
}
