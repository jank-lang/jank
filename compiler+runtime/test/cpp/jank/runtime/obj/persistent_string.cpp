#include <fmt/format.h>

#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_string")
  {
    persistent_string const s{ "foo bar" };
    auto const min{ make_box<integer>(0) };
    auto const min_char{ make_box<character>("f") };
    auto const mid{ make_box<integer>(3) };
    auto const mid_char{ make_box<character>(" ") };
    auto const max{ make_box<integer>(6) };
    auto const max_char{ make_box<character>("r") };
    auto const over{ make_box<integer>(7) };
    auto const under{ make_box<integer>(-1) };
    auto const nil{ nil::nil_const() };
    auto const non_int{ make_box<character>("z") };
    TEST_CASE("get")
    {
      CHECK(equal(s.get(min), min_char));
      CHECK(equal(s.get(mid), mid_char));
      CHECK(equal(s.get(max), max_char));
      CHECK(equal(s.get(over), nil));
      CHECK(equal(s.get(under), nil));
      CHECK(equal(s.get(non_int), nil));
    }
    TEST_CASE("get with fallback")
    {
      CHECK(equal(s.get(min, non_int), min_char));
      CHECK(equal(s.get(mid, non_int), mid_char));
      CHECK(equal(s.get(max, non_int), max_char));
      CHECK(equal(s.get(over, non_int), non_int));
      CHECK(equal(s.get(under, non_int), non_int));
      CHECK(equal(s.get(non_int, non_int), non_int));
    }
    TEST_CASE("contains")
    {
      CHECK(s.contains(min));
      CHECK(s.contains(mid));
      CHECK(s.contains(max));
      CHECK(!s.contains(over));
      CHECK(!s.contains(under));
      CHECK(!s.contains(non_int));
    }
    TEST_CASE("get_entry not implemented")
    {
      try
      {
        s.get_entry(min);
        CHECK(false);
      }
      catch(std::exception const &e)
      {
        auto const actual{ fmt::format("{}", e.what()) };
        auto const expected{ "get_entry not supported on string" };
        CHECK_EQ(actual, expected);
      }
    }
  };
}
