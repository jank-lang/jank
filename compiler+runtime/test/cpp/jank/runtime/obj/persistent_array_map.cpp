#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_array_map")
  {
    TEST_CASE("promotion to persistent_hash_map in assoc")
    {
      jank::runtime::detail::native_array_map data{};
      data.reserve(8);

      data.insert_unique(make_box('1'), make_box('1'));
      data.insert_unique(make_box('2'), make_box('2'));
      data.insert_unique(make_box('3'), make_box('3'));
      data.insert_unique(make_box('4'), make_box('4'));
      data.insert_unique(make_box('5'), make_box('5'));
      data.insert_unique(make_box('6'), make_box('6'));
      data.insert_unique(make_box('7'), make_box('7'));
      data.insert_unique(make_box('8'), make_box('8'));

      auto const v{ make_box<persistent_array_map>(data) };

      CHECK(v->obj_type == object_type::persistent_array_map);

      auto const pv{ v->assoc(make_box('9'), make_box('9')) };

      CHECK(pv->type == object_type::persistent_hash_map);
    }

    TEST_CASE("promotion to persistent_hash_map in conj")
    {
      jank::runtime::detail::native_array_map data{};
      data.reserve(8);

      data.insert_unique(make_box('1'), make_box('1'));
      data.insert_unique(make_box('2'), make_box('2'));
      data.insert_unique(make_box('3'), make_box('3'));
      data.insert_unique(make_box('4'), make_box('4'));
      data.insert_unique(make_box('5'), make_box('5'));
      data.insert_unique(make_box('6'), make_box('6'));
      data.insert_unique(make_box('7'), make_box('7'));
      data.insert_unique(make_box('8'), make_box('8'));

      auto const v{ make_box<persistent_array_map>(data) };

      CHECK(v->obj_type == object_type::persistent_array_map);

      auto const pv{ v->conj(
        make_box<persistent_vector>(std::in_place, make_box('9'), make_box('9'))) };

      CHECK(pv->type == object_type::persistent_hash_map);
    }
  }
}
