#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"
#include <jank/parse/expect/error/syntax/syntax.hpp>

namespace jank
{
  struct unicode_test
  { };
  using unicode_group = jest::group<unicode_test>;
  static unicode_group const unicode_obj{ "ident unicode" };
}

namespace jest
{
  template <> template <>
  void jank::unicode_group::test<0>()
  {
    expect_exception<jank::parse::expect::error::type::exception<>>
    ([]{ jank::common::translate("parse/ident/unicode/fail_not_supported.jank"); });
  }
}
