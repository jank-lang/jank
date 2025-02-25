#include <jank/runtime/core.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("core runtime")
  {
    TEST_CASE("equal")
    {
      CHECK(equal(nullptr, nullptr));
      CHECK(!equal(nullptr, make_box(42)));
      CHECK(!equal(make_box(42), nullptr));
    }
  }
}
