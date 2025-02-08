#include <jank/runtime/obj/lazy_sequence.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/core/make_box.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  TEST_SUITE("lazy_sequence")
  {
    TEST_CASE("equal")
    {
      //CHECK(equal(make_box<lazy_sequence>(make_box<native_function_wrapper>(convert_function([]() { return nullptr; }))),
      //            make_box<lazy_sequence>(make_box<native_function_wrapper>(convert_function([]() { return nullptr; })))));
    }
  }
}

