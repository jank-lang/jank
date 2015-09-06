#pragma once

#include <jest/jest.hpp>

#include "common/translate.hpp"

namespace jank
{
  struct first_class_test{ };
  using first_class_group = jest::group<first_class_test>;
  static first_class_group const first_class_obj{ "non-generic lambda as first-class objects" };
}

namespace jest
{
  template <> template <>
  void jank::first_class_group::test<0>()
  { jank::common::translate("translate/lambda/non-generic/first-class/pass_simple.jank"); }
}
