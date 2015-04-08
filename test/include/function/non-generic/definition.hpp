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
  { jank::common::run("function/non-generic/definition/pass_empty.jank"); }

  template <> template <>
  void jank::definition_group::test<1>()
  { jank::common::run("function/non-generic/definition/pass_primitive.jank"); }

  template <> template <>
  void jank::definition_group::test<2>()
  {
    expect_exception<jank::interpret::expect::error::syntax::syntax<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_missing_param_name.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<3>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<4>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_multiple_definition.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<5>()
  { jank::common::run("function/non-generic/definition/pass_return_primitive.jank"); }

  template <> template <>
  void jank::definition_group::test<6>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_invalid_return_type.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<7>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_no_param_list.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<8>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_no_return_type.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<9>()
  { jank::common::run("function/non-generic/definition/pass_return_single_value.jank"); }

  template <> template <>
  void jank::definition_group::test<10>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/definition/fail_return_incorrect_type.jank"); });
  }

  template <> template <>
  void jank::definition_group::test<11>()
  { jank::common::run("function/non-generic/definition/pass_body.jank"); }
}
