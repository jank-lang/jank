#pragma once

#include <jest/jest.hpp>

#include "common/run.hpp"

namespace jank
{
  struct call_test
  {
    void reset()
    { std::cout.rdbuf(out.rdbuf()); out.str(""); }
    std::stringstream out;
  };
  using call_group = jest::group<call_test>;
  static call_group const call_obj{ "non-generic function call" };
}

namespace jest
{
  template <> template <>
  void jank::call_group::test<0>()
  { jank::common::run("function/non-generic/call/pass_empty.jank"); }

  template <> template <>
  void jank::call_group::test<1>()
  {
    reset();
    jank::common::run("function/non-generic/call/pass_print.jank");
    expect_equal(out.str(), "all good\n");
  }

  template <> template <>
  void jank::call_group::test<2>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/call/fail_invalid_function.jank"); });
  }

  template <> template <>
  void jank::call_group::test<3>()
  {
    reset();
    jank::common::run("function/non-generic/call/pass_print_primitive.jank");
    expect_equal(out.str(), "42 3.14 ok\n");
  }

  template <> template <>
  void jank::call_group::test<4>()
  {
    expect_exception<jank::interpret::expect::error::type::overload>
    ([]{ jank::common::run("function/non-generic/call/fail_too_few_params.jank"); });
  }

  template <> template <>
  void jank::call_group::test<5>()
  {
    expect_exception<jank::interpret::expect::error::type::overload>
    ([]{ jank::common::run("function/non-generic/call/fail_too_many_params.jank"); });
  }

  template <> template <>
  void jank::call_group::test<6>()
  {
    expect_exception<jank::interpret::expect::error::type::overload>
    ([]{ jank::common::run("function/non-generic/call/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::call_group::test<7>()
  {
    reset();
    jank::common::run("function/non-generic/call/pass_chain.jank");
    expect_equal(out.str(), "77\n-5.05\ngood\n");
  }
}
