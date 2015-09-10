#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/exception.hpp>

namespace jank
{
  struct nested_test
  { };
  using nested_group = jest::group<nested_test>;
  static nested_group const nested_obj{ "comment nested" };
}

namespace jest
{
  template <> template <>
  void jank::nested_group::test<0>()
  { jank::common::translate("parse/comment/nested/pass_single_line.jank"); }

  template <> template <>
  void jank::nested_group::test<1>()
  { jank::common::translate("parse/comment/nested/pass_single_line_multi_start.jank"); }

  template <> template <>
  void jank::nested_group::test<2>()
  { jank::common::translate("parse/comment/nested/pass_multi_line.jank"); }

  template <> template <>
  void jank::nested_group::test<3>()
  { jank::common::translate("parse/comment/nested/pass_multi_line_multi_start.jank"); }

  template <> template <>
  void jank::nested_group::test<4>()
  { jank::common::translate("parse/comment/nested/pass_multi_line_multi_end.jank"); }

  template <> template <>
  void jank::nested_group::test<5>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/comment/nested/fail_single_line_multi_end.jank"); });
  }

  template <> template <>
  void jank::nested_group::test<6>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/comment/nested/fail_no_close.jank"); });
  }
}
