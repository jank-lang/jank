/* clang-format off */
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank
{
  TEST_SUITE("persistent_string")
  {
    TEST_CASE("Index")
    {
      SUBCASE("Present no default")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        CHECK(runtime::equal(s.get(runtime::make_box<runtime::obj::integer>(0)), (runtime::make_box<runtime::obj::character>("f"))));
      }

      SUBCASE("Absent no default")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        CHECK(runtime::equal(s.get(runtime::make_box<runtime::obj::integer>(7)), runtime::obj::nil::nil_const()));
      }

      SUBCASE("Present with default")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        CHECK(runtime::equal(s.get(runtime::make_box<runtime::obj::integer>(0), runtime::make_box<runtime::obj::character>("o")), runtime::make_box<runtime::obj::character>("f")));
      }

      SUBCASE("Absent with default")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        CHECK(runtime::equal(s.get(runtime::make_box<runtime::obj::integer>(0), runtime::make_box<runtime::obj::character>("o")), runtime::make_box<runtime::obj::character>("f")));
      }

      SUBCASE("Contains true")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        CHECK(s.contains(runtime::make_box<runtime::obj::integer>(0)));
      }

      SUBCASE("Contains false")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        CHECK(!s.contains(runtime::make_box<runtime::obj::integer>(7)));
      }

      SUBCASE("get_entry not supported")
      {
        runtime::obj::persistent_string const s{ "foo bar" };
        try
        {
          s.get_entry(runtime::make_box<runtime::obj::integer>(0));
          CHECK(false);
        }
        catch(std::exception const &e)
        {
          std::string const actual = e.what();
          std::string const expected = "get_entry not supported on string";
          CHECK_EQ(actual, expected);
        }
      }
    }
  };
}
