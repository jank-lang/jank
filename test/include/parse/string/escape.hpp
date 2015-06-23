#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/exception.hpp>

namespace jank
{
  struct string_escape_test
  { };
  using string_escape_group = jest::group<string_escape_test>;
  static string_escape_group const string_escape_obj{ "string escape" };
}

namespace jest
{
  template <> template <>
  void jank::string_escape_group::test<0>()
  { jank::common::translate("parse/string/escape/pass_escape_open.jank"); }

  template <> template <>
  void jank::string_escape_group::test<1>()
  { jank::common::translate("parse/string/escape/pass_escape_close.jank"); }

  template <> template <>
  void jank::string_escape_group::test<2>()
  { jank::common::translate("parse/string/escape/pass_escape_both.jank"); }

  template <> template <>
  void jank::string_escape_group::test<3>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/string/escape/fail_unescaped_open.jank"); });
  }

  template <> template <>
  void jank::string_escape_group::test<4>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/string/escape/fail_unescaped_close.jank"); });
  }

  template <> template <>
  void jank::string_escape_group::test<5>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/string/escape/fail_unescaped_both.jank"); });
  }

  template <> template <>
  void jank::string_escape_group::test<6>()
  {
    expect_exception<jank::parse::expect::error::syntax::exception<>>
    ([]{ jank::common::translate("parse/string/escape/fail_lots_of_unescaped_closes.jank"); });
  }
}
