#include <doctest/doctest.h>

#include <jank/runtime/obj/big_decimal.hpp>
#include <jank/runtime/core/equal.hpp>

using namespace jank::runtime;
using namespace jank::runtime::obj;

TEST_CASE("big_decimal creation and equality")
{
  auto const bd1 = big_decimal::create("123.45");
  auto const bd2 = big_decimal::create("123.45");
  auto const bd3 = big_decimal::create("123.46");

  CHECK(equal(bd1, bd2));
  CHECK(!equal(bd1, bd3));
}
