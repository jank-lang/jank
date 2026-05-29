#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/equal.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("integer_range")
  {
    TEST_CASE("equal")
    {
      CHECK(equal(integer_range::create(5), integer_range::create(5)));
      CHECK(equal(integer_range::create(1), integer_range::create(1)));
      CHECK(!equal(integer_range::create(6), integer_range::create(5)));
      CHECK(!equal(integer_range::create(5), integer_range::create(6)));
      CHECK(equal(integer_range::create(0), integer_range::create(0)));
      CHECK(!equal(integer_range::create(0), integer_range::create(5)));
      CHECK(!equal(integer_range::create(1), integer_range::create(0)));
    }
  }
}
