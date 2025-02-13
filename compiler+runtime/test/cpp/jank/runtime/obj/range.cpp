#include <jank/runtime/obj/range.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("range")
  {
    TEST_CASE("equal")
    {
      CHECK(equal(range::create(make_box(5)), range::create(make_box(5))));
      CHECK(equal(range::create(make_box(1)), range::create(make_box(1))));
      CHECK(!equal(range::create(make_box(6)), range::create(make_box(5))));
      CHECK(!equal(range::create(make_box(5)), range::create(make_box(6))));
      CHECK(equal(range::create(make_box(0)), range::create(make_box(0))));
      CHECK(!equal(range::create(make_box(0)), range::create(make_box(5))));
      CHECK(!equal(range::create(make_box(1)), range::create(make_box(0))));
    }
  }
}
