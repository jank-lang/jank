#pragma once

#include <jest/jest.hpp>

#include "common/run.hpp"

namespace jank
{
  struct calling_test
  {
    calling_test()
    { std::cout.rdbuf(out.rdbuf()); }
    void reset()
    { out.str(""); }
    std::stringstream out;
  };
  using calling_group = jest::group<calling_test>;
  static calling_group const calling_obj{ "non-generic function calling" };
}

namespace jest
{
  template <> template <>
  void jank::calling_group::test<0>()
  { jank::common::run("function/non-generic/calling/pass_empty.jank"); }

  template <> template <>
  void jank::calling_group::test<1>()
  {
    reset();
    jank::common::run("function/non-generic/calling/pass_print.jank");
    expect_equal(out.str(), "all good\n");
  }

  template <> template <>
  void jank::calling_group::test<2>()
  {
    expect_exception<jank::interpret::expect::error::type::type<>>
    ([]{ jank::common::run("function/non-generic/calling/fail_invalid_function.jank"); });
  }

  template <> template <>
  void jank::calling_group::test<3>()
  {
    reset();
    jank::common::run("function/non-generic/calling/pass_print_primitive.jank");
    expect_equal(out.str(), "42 3.14 ok\n");
  }

  template <> template <>
  void jank::calling_group::test<4>()
  {
    expect_exception<jank::interpret::expect::error::type::overload>
    ([]{ jank::common::run("function/non-generic/calling/fail_too_few_params.jank"); });
  }

  template <> template <>
  void jank::calling_group::test<5>()
  {
    expect_exception<jank::interpret::expect::error::type::overload>
    ([]{ jank::common::run("function/non-generic/calling/fail_too_many_params.jank"); });
  }

  template <> template <>
  void jank::calling_group::test<6>()
  {
    expect_exception<jank::interpret::expect::error::type::overload>
    ([]{ jank::common::run("function/non-generic/calling/fail_invalid_param_type.jank"); });
  }

  template <> template <>
  void jank::calling_group::test<7>()
  {
    reset();
    jank::common::run("function/non-generic/calling/pass_chain.jank");
    expect_equal(out.str(), "77\n-5.05\ngood\n");
  }
}
