#include <jank/debug/local_names.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::debug
{
  TEST_SUITE("debug::local_names")
  {
    TEST_CASE("round trip")
    {
      CHECK_EQ("foo-bar?", decode_local(encode_local("foo-bar?", 7)).unwrap());
      CHECK_EQ("foo", decode_local(encode_local("foo", 0)).unwrap());
      CHECK_EQ("a_b", decode_local(encode_local("a_b", 12)).unwrap());
    }

    TEST_CASE("shadowed names stay unique")
    {
      CHECK_NE(encode_local("x", 1), encode_local("x", 2));
    }

    TEST_CASE("ordinary identifiers are not decoded")
    {
      CHECK(decode_local("foo_bar_QMARK_").is_none());
      CHECK(decode_local("v12").is_none());
      CHECK(decode_local("_jd_Zzz_1").is_none());
      CHECK(decode_local("_jd_Z4_1").is_none());
    }
  }
}
