#pragma once

#include <jest/jest.hpp>

#include "common/run.hpp"

namespace jank
{
  struct definition_test
  {
    definition_test()
    { std::cout.rdbuf(out.rdbuf()); }
    std::stringstream out;
  };
  using definition_group = jest::group<definition_test>;
  static definition_group const definition_obj{ "non-generic function definition" };
}

namespace jest
{
  template <> template <>
  void jank::definition_group::test<0>()
  {
    jank::common::run("test/src/jank/function/non-generic/definition/pass_primitive.jank");
  }

  template <> template <>
  void jank::definition_group::test<1>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("test/src/jank/function/non-generic/definition/fail_multiple_definition.jank"); });
  }
}
