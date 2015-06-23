#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/exception.hpp>

namespace jank
{
  struct single_line_test
  { };
  using single_line_group = jest::group<single_line_test>;
  static single_line_group const single_line_obj{ "comment single line" };
}

namespace jest
{
  template <> template <>
  void jank::single_line_group::test<0>()
  { jank::common::translate("parse/comment/single_line/pass_normal.jank"); }

  template <> template <>
  void jank::single_line_group::test<1>()
  { jank::common::translate("parse/comment/single_line/pass_unicode.jank"); }

  template <> template <>
  void jank::single_line_group::test<2>()
  { jank::common::translate("parse/comment/single_line/pass_parens.jank"); }

  template <> template <>
  void jank::single_line_group::test<3>()
  { jank::common::translate("parse/comment/single_line/pass_quotes.jank"); }

  template <> template <>
  void jank::single_line_group::test<4>()
  { jank::common::translate("parse/comment/single_line/pass_double_close.jank"); }

  template <> template <>
  void jank::single_line_group::test<5>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/comment/single_line/fail_no_close.jank"); });
  }
}
