#include <jank/util/sha256.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::util
{
  TEST_SUITE("util::sha256")
  {
    // Validated with `sha256sum` from GNU coreutils
    TEST_CASE("empty")
    {
      CHECK_EQ("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
               util::sha256(""));
    }
    TEST_CASE("with data")
    {
      CHECK_EQ("c3ab8ff13720e8ad9047dd39466b3c8974e592c2fa383d4a3960714caef0c4f2",
               util::sha256("foobar"));
    }
  }
}
