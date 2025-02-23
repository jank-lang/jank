#include <jank/runtime/obj/repeat.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("repeat")
  {
    TEST_CASE("equal")
    {
      CHECK(
        equal(repeat::create(make_box(5), make_box(5)), repeat::create(make_box(5), make_box(5))));
      CHECK(
        equal(repeat::create(make_box(1), make_box(1)), repeat::create(make_box(1), make_box(1))));
      CHECK(
        !equal(repeat::create(make_box(6), make_box(5)), repeat::create(make_box(5), make_box(5))));
      CHECK(
        !equal(repeat::create(make_box(5), make_box(5)), repeat::create(make_box(5), make_box(6))));
      CHECK(
        equal(repeat::create(make_box(0), make_box(0)), repeat::create(make_box(0), make_box(0))));
      CHECK(
        !equal(repeat::create(make_box(0), make_box(0)), repeat::create(make_box(5), make_box(0))));
      CHECK(
        !equal(repeat::create(make_box(1), make_box(1)), repeat::create(make_box(0), make_box(1))));

      CHECK(equal(repeat::create(make_box(0), make_box(0)), persistent_list::empty()));
      CHECK(equal(seq(repeat::create(make_box(0), make_box(0))), nil::nil_const()));
    }
    TEST_CASE("seq")
    {
      CHECK(equal(seq(repeat::create(make_box(0), make_box(0))), nil::nil_const()));
    }
  }
}
