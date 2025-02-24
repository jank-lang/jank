#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("integer_range")
  {
    TEST_CASE("equal")
    {
      CHECK(equal(integer_range::create(make_box(5)), integer_range::create(make_box(5))));
      CHECK(equal(integer_range::create(make_box(1)), integer_range::create(make_box(1))));
      CHECK(!equal(integer_range::create(make_box(6)), integer_range::create(make_box(5))));
      CHECK(!equal(integer_range::create(make_box(5)), integer_range::create(make_box(6))));
      CHECK(equal(integer_range::create(make_box(0)), integer_range::create(make_box(0))));
      CHECK(!equal(integer_range::create(make_box(0)), integer_range::create(make_box(5))));
      CHECK(!equal(integer_range::create(make_box(1)), integer_range::create(make_box(0))));
    }
  }
}
