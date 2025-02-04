#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_vector")
  {
    static auto const v{
      make_box<persistent_vector>(std::in_place,
          make_box('f'),
          make_box('o'),
          make_box('o'),
          make_box(' '),
          make_box('b'),
          make_box('a'),
          make_box('r')) };
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
      CHECK(equal(find(v, min),
                  make_box<persistent_vector>(std::in_place, min, min_char)));
      CHECK(equal(find(v, mid),
                  make_box<persistent_vector>(std::in_place, mid, mid_char)));
      CHECK(equal(find(v, max),
                  make_box<persistent_vector>(std::in_place, max, max_char)));
      CHECK(equal(find(v, over), nil));
      CHECK(equal(find(v, under), nil));
      CHECK(equal(find(v, non_int), nil));
    }
  }
}
