#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/context.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_vector")
  {
    static persistent_vector_ptr const v(persistent_vector::create(nullptr)
                                           ->conj(make_box('f'))
                                           ->conj(make_box('o'))
                                           ->conj(make_box('o'))
                                           ->conj(make_box(' '))
                                           ->conj(make_box('b'))
                                           ->conj(make_box('a'))
                                           ->conj(make_box('r')));
    static auto const min{ make_box(0) };
    static auto const min_char{ make_box('f') };
    static auto const mid{ make_box(3) };
    static auto const mid_char{ make_box(' ') };
    static auto const max{ make_box(6) };
    static auto const max_char{ make_box('r') };
    static auto const over{ make_box(7) };
    static auto const under{ make_box(-1) };
    static auto const nil{ nil::nil_const() };
    static auto const non_int{ make_box('z') };

    TEST_CASE("get")
    {
      CHECK(equal(get(v, min), min_char));
      CHECK(equal(get(v, mid), mid_char));
      CHECK(equal(get(v, max), max_char));
      CHECK(equal(get(v, over), nil));
      CHECK(equal(get(v, under), nil));
      CHECK(equal(get(v, non_int), nil));
    }
    TEST_CASE("get with fallback")
    {
      CHECK(equal(get(v, min, non_int), min_char));
      CHECK(equal(get(v, mid, non_int), mid_char));
      CHECK(equal(get(v, max, non_int), max_char));
      CHECK(equal(get(v, over, non_int), non_int));
      CHECK(equal(get(v, under, non_int), non_int));
      CHECK(equal(get(v, non_int, non_int), non_int));
    }
    TEST_CASE("contains")
    {
      CHECK(contains(v, min));
      CHECK(contains(v, mid));
      CHECK(contains(v, max));
      CHECK(!contains(v, over));
      CHECK(!contains(v, under));
      CHECK(!contains(v, non_int));
    }
    TEST_CASE("find")
    {
      CHECK(
        equal(find(v, min),
              persistent_vector::create(nullptr)->conj(make_box(min))->conj(make_box(min_char))));
      CHECK(
        equal(find(v, mid),
              persistent_vector::create(nullptr)->conj(make_box(mid))->conj(make_box(mid_char))));
      CHECK(
        equal(find(v, max),
              persistent_vector::create(nullptr)->conj(make_box(max))->conj(make_box(max_char))));
      CHECK(equal(find(v, over), nil));
      CHECK(equal(find(v, under), nil));
      CHECK(equal(find(v, non_int), nil));
    }
  }
}
