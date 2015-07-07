#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct nest_test
  {
    void reset()
    { std::cout.rdbuf(out.rdbuf()); out.str(""); }
    std::stringstream out;
  };
  using nest_group = jest::group<nest_test>;
  static nest_group const nest_obj{ "non-generic function nest" };
}

namespace jest
{
  template <> template <>
  void jank::nest_group::test<0>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_define.jank");
    expect_equal(out.str(), "all good\n");
  }

  template <> template <>
  void jank::nest_group::test<1>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_redefine_outer.jank");
    expect_equal(out.str(), "redefined\n");
  }

  template <> template <>
  void jank::nest_group::test<2>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_overload_outer.jank");
    expect_equal(out.str(), "42.7\n");
  }

  template <> template <>
  void jank::nest_group::test<3>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_overload_self.jank");
    expect_equal(out.str(), "all good\n");
  }

  template <> template <>
  void jank::nest_group::test<4>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_overload_inner.jank");
    expect_equal(out.str(), "integer\n");
  }

  template <> template <>
  void jank::nest_group::test<5>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_capture_params.jank");
    expect_equal(out.str(), "true\n");
  }

  template <> template <>
  void jank::nest_group::test<6>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_redefine_self.jank");
    expect_equal(out.str(), "inner\n");
  }

  template <> template <>
  void jank::nest_group::test<7>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/nest/pass_overload_outer_call_outer.jank");
    expect_equal(out.str(), "42\n");
  }
}
