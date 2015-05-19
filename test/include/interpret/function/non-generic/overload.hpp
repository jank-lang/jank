#pragma once

#include <jest/jest.hpp>

#include "common/interpret.hpp"

namespace jank
{
  struct overload_test
  {
    void reset()
    { std::cout.rdbuf(out.rdbuf()); out.str(""); }
    std::stringstream out;
  };
  using overload_group = jest::group<overload_test>;
  static overload_group const overload_obj{ "non-generic function overload" };
}

namespace jest
{
  template <> template <>
  void jank::overload_group::test<0>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/overload/pass_different_param_count.jank");
    expect_equal(out.str(), "integer and real\ninteger\nnullary\n");
  }

  template <> template <>
  void jank::overload_group::test<1>()
  {
    reset();
    jank::common::interpret("interpret/function/non-generic/overload/pass_same_param_count.jank");
    expect_equal(out.str(), "integer\nreal\nstring and boolean\nboolean and string\n");
  }
}
