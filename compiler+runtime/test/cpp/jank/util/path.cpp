#include <jank/util/path.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::util
{
  TEST_SUITE("util::path")
  {
    TEST_CASE("compact_path")
    {
      SUBCASE("empty")
      {
        CHECK_EQ("", util::compact_path("", 5));
        CHECK_EQ("", util::compact_path("", 0));
      }

      SUBCASE("less")
      {
        CHECK_EQ("a.cpp", util::compact_path("a.cpp", 6));
      }

      SUBCASE("equal")
      {
        CHECK_EQ("a.cpp", util::compact_path("a.cpp", 5));
      }

      SUBCASE("greater")
      {
        CHECK_EQ("….cpp", util::compact_path("abc.cpp", 5));
        CHECK_EQ("…c.cpp", util::compact_path("abc.cpp", 6));

        CHECK_EQ("…like-so", util::compact_path("some-long-file-name-like-so", 8));
      }
    }
  }
}
