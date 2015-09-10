#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/exception.hpp>

namespace jank
{
  struct multi_line_test
  { };
  using multi_line_group = jest::group<multi_line_test>;
  static multi_line_group const multi_line_obj{ "comment multi line" };
}

namespace jest
{
  template <> template <>
  void jank::multi_line_group::test<0>()
  { jank::common::translate("parse/comment/multi_line/pass_normal.jank"); }

  template <> template <>
  void jank::multi_line_group::test<1>()
  { jank::common::translate("parse/comment/multi_line/pass_unicode.jank"); }

  template <> template <>
  void jank::multi_line_group::test<2>()
  { jank::common::translate("parse/comment/multi_line/pass_parens.jank"); }

  template <> template <>
  void jank::multi_line_group::test<3>()
  { jank::common::translate("parse/comment/multi_line/pass_quotes.jank"); }

  template <> template <>
  void jank::multi_line_group::test<4>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/comment/multi_line/fail_double_close.jank"); });
  }

  template <> template <>
  void jank::multi_line_group::test<5>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/comment/multi_line/fail_no_close.jank"); });
  }
}
