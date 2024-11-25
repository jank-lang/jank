#include <unistd.h>

#include <jank/runtime/detail/native_persistent_list.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::detail
{
  TEST_SUITE("native_persistent_list")
  {
    TEST_CASE("Empty")
    {
      native_persistent_list_impl<int> l;
      CHECK(l.first().is_none());
      CHECK(l.rest().size() == 0);
      CHECK(l.size() == 0);
    }

    TEST_CASE("Unit")
    {
      native_persistent_list_impl<int> l{ 10 };
      CHECK(l.first() == 10);
      CHECK(l.rest().size() == 0);
      CHECK(l.size() == 1);
    }

    TEST_CASE("Cons")
    {
      native_persistent_list_impl<int> l1{ 10 };
      native_persistent_list_impl<int> l2{ l1.conj(20) };
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
        native_persistent_list_impl<int> l;
        for(auto i : l)
        {
          static_cast<void>(i);
          CHECK(false);
        }
      }

      SUBCASE("Non-empty")
      {
        native_persistent_list_impl<int> l{ 10, 20, 30 };
        int c{ 1 };
        for(auto i : l)
        {
          CHECK(i == c++ * 10);
        }
        /* Check twice to ensure iterators aren't stateful. */
        c = 1;
        for(auto i : l)
        {
          CHECK(i == c++ * 10);
        }
      }
    }
  }
}
