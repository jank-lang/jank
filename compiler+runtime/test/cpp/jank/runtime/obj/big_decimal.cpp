#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/obj/big_decimal.hpp>
#include <jank/runtime/obj/big_integer.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/ratio.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

using namespace jank::runtime;
using namespace jank::runtime::obj;

TEST_CASE("big_decimal creation and equality")
{
  auto const bd1(big_decimal::create("123.45"));
  auto const bd2(big_decimal::create("123.45"));
  auto const bd3(big_decimal::create("123.46"));

  CHECK(equal(bd1, bd2));
  CHECK(!equal(bd1, bd3));
}

TEST_CASE("numeric equality preserves numeric categories")
{
  auto const check_symmetric_equal
    = [](object_ref const lhs, object_ref const rhs, bool const expected) {
        CHECK(equal(lhs, rhs) == expected);
        /* NOLINTNEXTLINE(readability-suspicious-call-argument) */
        CHECK(equal(rhs, lhs) == expected);
      };

  auto const integer_value{ make_box<integer>(1LL) };
  auto const small_integer_value{ make_box(1) };
  auto const big_integer_value{ make_box<big_integer>(1LL) };
  auto const real_value{ make_box<real>(1.0) };
  auto const small_real_value{ make_box(1.0) };
  auto const ratio_value{ make_box<obj::ratio>(ratio_data(1, 1)) };
  auto const other_ratio_value{ make_box<obj::ratio>(ratio_data(2, 2)) };
  auto const decimal_value{ big_decimal::create("1.0") };
  auto const other_decimal_value{ big_decimal::create("1.00") };

  SUBCASE("same numeric categories remain equal")
  {
    check_symmetric_equal(integer_value, small_integer_value, true);
    check_symmetric_equal(integer_value, big_integer_value, true);
    check_symmetric_equal(real_value, small_real_value, true);
    check_symmetric_equal(ratio_value, other_ratio_value, true);
    check_symmetric_equal(decimal_value, other_decimal_value, true);
  }

  SUBCASE("different numeric categories are not equal")
  {
    check_symmetric_equal(integer_value, real_value, false);
    check_symmetric_equal(integer_value, ratio_value, false);
    check_symmetric_equal(integer_value, decimal_value, false);
    check_symmetric_equal(real_value, ratio_value, false);
    check_symmetric_equal(real_value, decimal_value, false);
    check_symmetric_equal(ratio_value, decimal_value, false);
  }

  SUBCASE("nested values preserve numeric categories")
  {
    auto const integers{ make_box<persistent_vector>(std::in_place, make_box(0), make_box(0)) };
    auto const reals{ make_box<persistent_vector>(std::in_place, make_box(0.0), make_box(0.0)) };

    check_symmetric_equal(integers, reals, false);
  }
}
