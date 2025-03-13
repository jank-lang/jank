#include <jank/util/fmt.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::util
{
  TEST_SUITE("fmt")
  {
    TEST_CASE("empty")
    {
      CHECK_EQ("", util::format(""));
    }

    TEST_CASE("no format specifier")
    {
      CHECK_EQ("foo bar spam", util::format("foo bar spam"));
    }

    TEST_CASE("args, with no format specifier")
    {
      CHECK_EQ("foo bar spam", util::format("foo bar spam", 1, 2, 3));
    }

    TEST_CASE("args, with format specifier")
    {
      CHECK_EQ("foo 1 bar 2 spam 3", util::format("foo {} bar {} spam {}", 1, 2, 3));
    }

    TEST_CASE("no args, with format specifier")
    {
      CHECK_THROWS(util::format("foo {}"));
    }

    TEST_CASE("just a format specifier")
    {
      CHECK_EQ("meow meow", util::format("{}", "meow meow"));
    }

    TEST_CASE("data after format specifier")
    {
      CHECK_EQ("foo_bar_spam", util::format("foo_{}_spam", "bar"));
    }
  }
}
