#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

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
  { jank::common::interpret("function/non-generic/call/pass_empty.jank"); }

  template <> template <>
  void jank::call_group::test<1>()
  {
    reset();
    jank::common::interpret("function/non-generic/call/pass_print.jank");
    expect_equal(out.str(), "all good\n");
  }

  template <> template <>
  void jank::call_group::test<2>()
  {
    reset();
    jank::common::interpret("function/non-generic/call/pass_print_primitive.jank");
    expect_equal(out.str(), "42 3.14 ok\n");
  }

  template <> template <>
  void jank::call_group::test<3>()
  {
    reset();
    jank::common::interpret("function/non-generic/call/pass_chain.jank");
    expect_equal(out.str(), "77\n-5.05\ngood\n");
  }
}
