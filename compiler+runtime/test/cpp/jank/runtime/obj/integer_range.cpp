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
      CHECK(equal(make_box<integer_range>(make_box(5)), make_box<integer_range>(make_box(5))));
      CHECK(equal(make_box<integer_range>(make_box(1)), make_box<integer_range>(make_box(1))));
      CHECK(!equal(make_box<integer_range>(make_box(6)), make_box<integer_range>(make_box(5))));
      CHECK(!equal(make_box<integer_range>(make_box(5)), make_box<integer_range>(make_box(6))));
      CHECK(equal(make_box<integer_range>(make_box(0)), make_box<integer_range>(make_box(0))));
      CHECK(!equal(make_box<integer_range>(make_box(0)), make_box<integer_range>(make_box(5))));
      //TODO empty integer_range uses persistent_list
      //CHECK(!equal(make_box<integer_range>(make_box(1)), make_box<integer_range>(make_box(0))));
    }
  }
}
