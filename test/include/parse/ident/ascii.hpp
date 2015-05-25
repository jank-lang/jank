#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/syntax.hpp>

namespace jank
{
  struct ascii_test
  { };
  using ascii_group = jest::group<ascii_test>;
  static ascii_group const ascii_obj{ "ident ascii" };
}

namespace jest
{
  template <> template <>
  void jank::ascii_group::test<0>()
  { jank::common::translate("parse/ident/ascii/pass_good_chars.jank"); }

  template <> template <>
  void jank::ascii_group::test<1>()
  {
    expect_exception<jank::translate::expect::error::type::exception<>>
    ([]{ jank::common::translate("parse/ident/ascii/fail_bad_chars.jank"); });
  }
}
