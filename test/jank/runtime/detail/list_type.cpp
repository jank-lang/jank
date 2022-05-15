#include <doctest/doctest.h>
#include <unistd.h>

#include <vector>
#include <array>
#include <iostream>

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/list_type.hpp>

namespace jank::runtime::detail
{
  TEST_CASE("Empty")
  {
    list_type_impl<int> l;
    CHECK(!l.first());
    CHECK(l.rest().size() == 0);
    CHECK(l.size() == 0);
  }

  TEST_CASE("Unit")
  {
    list_type_impl<int> l{ 10 };
    CHECK(l.first() == 10);
    CHECK(l.rest().size() == 0);
    CHECK(l.size() == 1);
  }

  TEST_CASE("Cons")
  {
    list_type_impl<int> l1{ 10 };
    list_type_impl<int> l2{ l1.cons(20) };
    CHECK(l2.first() == 20);
    CHECK(l2.size() == 2);
    CHECK(l2.rest().size() == 1);
    CHECK(l2.rest().first() == 10);
    CHECK(l2.rest().rest().size() == 0);
  }

  TEST_CASE("Iterator")
  {
    SUBCASE("Empty")
    {
      list_type_impl<int> l;
      for(auto i : l)
      {
        static_cast<void>(i);
        CHECK(false);
      }
    }

    SUBCASE("Non-empty")
    {
      list_type_impl<int> l{ 10, 20, 30 };
      int c{ 1 };
      for(auto i : l)
      { CHECK(i == c++ * 10); }
      /* Check twice to ensure iterators aren't stateful. */
      c = 1;
      for(auto i : l)
      { CHECK(i == c++ * 10); }
    }
  }
}
