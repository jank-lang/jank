#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/equal.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_list")
  {
    TEST_CASE("equal")
    {
      CHECK(equal(make_box<persistent_list>(std::in_place).erase(),
                  make_box<persistent_list>(std::in_place).erase()));
      CHECK(!equal(make_box<persistent_list>(std::in_place).erase(),
                   make_box<persistent_list>(std::in_place, make_box('f'), make_box('o')).erase()));
      CHECK(!equal(make_box<persistent_list>(std::in_place, make_box('f'), make_box('o')).erase(),
                   make_box<persistent_list>(std::in_place).erase()));
      CHECK(equal(make_box<persistent_list>(std::in_place, make_box('f'), make_box('o')).erase(),
                  make_box<persistent_list>(std::in_place, make_box('f'), make_box('o')).erase()));
      CHECK(!equal(make_box<persistent_list>(std::in_place, make_box('f')).erase(),
                   make_box<persistent_list>(std::in_place, make_box('f'), make_box('o')).erase()));
      CHECK(!equal(make_box<persistent_list>(std::in_place, make_box('f'), make_box('o')).erase(),
                   make_box<persistent_list>(std::in_place, make_box('f')).erase()));
    }
  }
}
