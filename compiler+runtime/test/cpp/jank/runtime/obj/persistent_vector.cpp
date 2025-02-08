#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_vector")
  {
    static auto const v{ make_box<persistent_vector>(std::in_place,
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
      CHECK(equal(v->get(min), min_char));
      CHECK(equal(v->get(mid), mid_char));
      CHECK(equal(v->get(max), max_char));
      CHECK(equal(v->get(over), nil));
      CHECK(equal(v->get(under), nil));
      CHECK(equal(v->get(non_int), nil));
    }
    TEST_CASE("get with fallback")
    {
      CHECK(equal(v->get(min, non_int), min_char));
      CHECK(equal(v->get(mid, non_int), mid_char));
      CHECK(equal(v->get(max, non_int), max_char));
      CHECK(equal(v->get(over, non_int), non_int));
      CHECK(equal(v->get(under, non_int), non_int));
      CHECK(equal(v->get(non_int, non_int), non_int));
    }
    TEST_CASE("contains")
    {
      CHECK(v->contains(min));
      CHECK(v->contains(mid));
      CHECK(v->contains(max));
      CHECK(!v->contains(over));
      CHECK(!v->contains(under));
      CHECK(!v->contains(non_int));
    }
    TEST_CASE("get_entry")
    {
      CHECK(equal(v->get_entry(min), make_box<persistent_vector>(std::in_place, min, min_char)));
      CHECK(equal(v->get_entry(mid), make_box<persistent_vector>(std::in_place, mid, mid_char)));
      CHECK(equal(v->get_entry(max), make_box<persistent_vector>(std::in_place, max, max_char)));
      CHECK(equal(v->get_entry(over), nil));
      CHECK(equal(v->get_entry(under), nil));
      CHECK(equal(v->get_entry(non_int), nil));
    }
    TEST_CASE("equal")
    {
      CHECK(equal(make_box<persistent_vector>(std::in_place),
                  make_box<persistent_vector>(std::in_place)));
      CHECK(!equal(make_box<persistent_vector>(std::in_place),
                   make_box<persistent_vector>(std::in_place, make_box('f'), make_box('o'))));
      CHECK(!equal(make_box<persistent_vector>(std::in_place, make_box('f'), make_box('o')),
                   make_box<persistent_vector>(std::in_place)));
      CHECK(equal(make_box<persistent_vector>(std::in_place, make_box('f'), make_box('o')),
                  make_box<persistent_vector>(std::in_place, make_box('f'), make_box('o'))));
      CHECK(!equal(make_box<persistent_vector>(std::in_place, make_box('f')),
                   make_box<persistent_vector>(std::in_place, make_box('f'), make_box('o'))));
      CHECK(!equal(make_box<persistent_vector>(std::in_place, make_box('f'), make_box('o')),
                   make_box<persistent_vector>(std::in_place, make_box('f'))));
    }
  }
}
