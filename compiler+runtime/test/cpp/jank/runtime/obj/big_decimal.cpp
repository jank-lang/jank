#include <doctest/doctest.h>

#include <jank/runtime/obj/big_decimal.hpp>
#include <jank/runtime/core/equal.hpp>

using namespace jank::runtime;
using namespace jank::runtime::obj;

TEST_CASE("big_decimal creation and equality") {
  auto bd1 = big_decimal::create("123.45");
  auto bd2 = big_decimal::create("123.45M");
  auto bd3 = big_decimal::create("123.46M");

  CHECK(equal(bd1, bd2));
  CHECK(!equal(bd1, bd3));
}
