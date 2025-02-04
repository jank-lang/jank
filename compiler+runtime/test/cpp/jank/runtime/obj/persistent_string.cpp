#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/core.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("persistent_string")
  {
    TEST_CASE("to_code_string")
    {
      static auto const s{ make_box("?") };
      static auto const expected{ make_box("\"?\"") };
      CHECK(equal(make_box(s->to_code_string()), expected));
    }
  }
}
