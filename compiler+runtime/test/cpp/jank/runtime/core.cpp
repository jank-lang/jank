#include <jank/runtime/core.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("core runtime")
  {
    TEST_CASE("subs")
    {
      auto const s{ make_box<obj::persistent_string>("foo bar") };
      CHECK(equal(subs(s, make_box(0)), make_box<obj::persistent_string>("foo bar")));
      CHECK(equal(subs(s, make_box(1)), make_box<obj::persistent_string>("oo bar")));
      CHECK(equal(subs(s, make_box(2)), make_box<obj::persistent_string>("o bar")));
      CHECK(equal(subs(s, make_box(0), make_box(7)), make_box<obj::persistent_string>("foo bar")));
      CHECK(equal(subs(s, make_box(0), make_box(0)), make_box<obj::persistent_string>("")));
      CHECK(equal(subs(s, make_box(1), make_box(1)), make_box<obj::persistent_string>("")));
      CHECK(equal(subs(s, make_box(1), make_box(6)), make_box<obj::persistent_string>("oo ba")));
      CHECK(equal(subs(s, make_box(1), make_box(7)), make_box<obj::persistent_string>("oo bar")));
      CHECK(equal(subs(s, make_box(3), make_box(4)), make_box<obj::persistent_string>(" ")));
    }

    TEST_CASE("equal")
    {
      CHECK(equal(nullptr, nullptr));
      CHECK(!equal(nullptr, make_box(42)));
      CHECK(!equal(make_box(42), nullptr));
    }
  }
}
