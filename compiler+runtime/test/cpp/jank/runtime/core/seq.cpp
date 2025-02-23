#include <jank/runtime/core.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_list.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::core
{
  TEST_SUITE("core runtime for seq")
  {
    TEST_CASE("sequence_equal")
    {
      CHECK(sequence_equal(obj::nil::nil_const(), make_box<obj::persistent_vector>()));
      CHECK(sequence_equal(make_box<obj::persistent_vector>(), obj::nil::nil_const()));
      CHECK(sequence_equal(make_box<obj::persistent_vector>(), obj::nil::nil_const()));
      CHECK(sequence_equal(make_box<obj::persistent_vector>(std::in_place, make_box('f')),
                           make_box<obj::persistent_list>(std::in_place, make_box('f'))));
      CHECK(!sequence_equal(make_box<obj::persistent_vector>(std::in_place, make_box('f')),
                            make_box<obj::persistent_list>(std::in_place, make_box('g'))));
      CHECK(!sequence_equal(
        make_box<obj::persistent_vector>(std::in_place, make_box('f'), make_box('g')),
        make_box<obj::persistent_list>(std::in_place, make_box('g'), make_box('f'))));
      CHECK(sequence_equal(
        make_box<obj::persistent_vector>(std::in_place, make_box('f'), make_box('g')),
        make_box<obj::persistent_list>(std::in_place, make_box('f'), make_box('g'))));
      CHECK(!sequence_equal(
        make_box<obj::persistent_vector>(std::in_place, make_box('f')),
        make_box<obj::persistent_list>(std::in_place, make_box('g'), make_box('f'))));
      CHECK(!sequence_equal(
        make_box<obj::persistent_vector>(std::in_place, make_box('f'), make_box('g')),
        make_box<obj::persistent_list>(std::in_place, make_box('g'))));
    }
  }
}
