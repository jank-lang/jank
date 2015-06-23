#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/exception.hpp>

namespace jank
{
  struct match_test
  { };
  using match_group = jest::group<match_test>;
  static match_group const match_obj{ "paren match" };
}

namespace jest
{
  template <> template <>
  void jank::match_group::test<0>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/paren/match/fail_open_nothing_else.jank"); });
  }

  template <> template <>
  void jank::match_group::test<1>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/paren/match/fail_multiple_open_nothing_else.jank"); });
  }

  template <> template <>
  void jank::match_group::test<2>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/paren/match/fail_close_nothing_else.jank"); });
  }

  template <> template <>
  void jank::match_group::test<3>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/paren/match/fail_multiple_close_nothing_else.jank"); });
  }
}
