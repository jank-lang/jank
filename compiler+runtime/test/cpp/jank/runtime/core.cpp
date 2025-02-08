#include <jank/runtime/core.hpp>
#include <jank/runtime/obj/persistent_string.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("jank::runtime::core")
  {
    TEST_CASE("subs")
    {
      auto const s{ make_box<obj::persistent_string>("foo bar") };
      CHECK(equal(subs(s, make_box(0)), make_box<obj::persistent_string>("foo bar")));
      CHECK(equal(subs(s, make_box(1)), make_box<obj::persistent_string>("oo bar")));
      CHECK(equal(subs(s, make_box(2)), make_box<obj::persistent_string>("o bar")));
      CHECK(equal(subs(s, make_box(0), make_box(0)), make_box<obj::persistent_string>("foo bar")));
      CHECK(equal(subs(s, make_box(1), make_box(1)), make_box<obj::persistent_string>("oo bar")));
      CHECK(equal(subs(s, make_box(2), make_box(2)), make_box<obj::persistent_string>("o bar")));
    }
  }
}
