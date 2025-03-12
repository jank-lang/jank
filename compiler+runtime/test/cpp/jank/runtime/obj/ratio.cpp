#include <stdexcept>
#include <cmath>

#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/rtti.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("ratio")
  {
    TEST_SUITE("ratio data")
    {
      TEST_CASE("Ratio data constructor")
      {
        SUBCASE("Valid ratio")
        {
          obj::ratio_data const ratio{ 4, 6 };
          CHECK_EQ(ratio.numerator, 2); // Simplified
          CHECK_EQ(ratio.denominator, 3);
        }
        SUBCASE("Invalid Denominator")
        {
          CHECK_THROWS_AS(obj::ratio_data(4, 0), std::invalid_argument);
        }
      }

      TEST_CASE("Real conversion")
      {
        obj::ratio_data const ratio{ 3, 4 };
        CHECK_EQ(ratio.to_real(), doctest::Approx(0.75));
      }

      TEST_CASE("Simplify functionality")
      {
        obj::ratio_data const ratio{ 10, -20 };
        CHECK_EQ(ratio.numerator, -1);
        CHECK_EQ(ratio.denominator, 2);
      }
    }

    TEST_CASE("Create ratio or integer")
    {
      SUBCASE("Simplify to integer")
      {
        auto const ratio_ptr{ obj::ratio::create(4, 2) };
        CHECK_EQ(ratio_ptr->type, object_type::integer);
        REQUIRE(ratio_ptr != nullptr);
        CHECK_EQ(expect_object<obj::integer>(ratio_ptr)->data, 2);
      }

      SUBCASE("Remain as ratio")
      {
        auto const ratio_ptr{ obj::ratio::create(3, 4) };
        auto const ratio{ expect_object<obj::ratio>(ratio_ptr) };
        REQUIRE(ratio != nullptr);
        CHECK_EQ(ratio->data.numerator, 3);
        CHECK_EQ(ratio->data.denominator, 4);
        CHECK_EQ(ratio->to_real(), doctest::Approx(0.75));
      }
    }

    TEST_CASE("Arithmetic operations")
    {
      obj::ratio_data const a{ 1, 2 }, b{ 1, 3 };

      SUBCASE("Addition")
      {
        auto const result{ *expect_object<obj::ratio>(a + b) };
        CHECK_EQ(result.data.numerator, 5);
        CHECK_EQ(result.data.denominator, 6);
      }
      SUBCASE("Subtraction")
      {
        auto const result{ *expect_object<obj::ratio>(a - b) };
        CHECK_EQ(result.data.numerator, 1);
        CHECK_EQ(result.data.denominator, 6);
      }
      SUBCASE("Multiplication")
      {
        auto const result{ *expect_object<obj::ratio>(a * b) };
        CHECK_EQ(result.data.numerator, 1);
        CHECK_EQ(result.data.denominator, 6);
      }

      SUBCASE("Division")
      {
        auto const result{ *expect_object<obj::ratio>(a / b) };
        CHECK_EQ(result.data.numerator, 3);
        CHECK_EQ(result.data.denominator, 2);
      }
    }

    TEST_CASE("Comparison operations")
    {
      obj::ratio_data const a{ 1, 2 }, b{ 2, 4 }, c{ 1, 3 };

      SUBCASE("Equality")
      {
        CHECK_EQ(a, b);
        CHECK_NE(a, c);
      }
      SUBCASE("Less Than")
      {
        CHECK_LT(c, a);
        CHECK_FALSE(a < b);
      }
      SUBCASE("Greater Than")
      {
        CHECK_GT(a, c);
        CHECK_FALSE(c > a);
      }
    }

    TEST_CASE("Edge cases and utility functions")
    {
      obj::ratio_data const ratio{ 6, -9 };

      SUBCASE("Simplify on negative denominator")
      {
        CHECK_EQ(ratio.numerator, -2);
        CHECK_EQ(ratio.denominator, 3);
      }
      SUBCASE("To string")
      {
        auto const ratio_ptr{ obj::ratio::create(3, 4) };
        auto const ratio{ expect_object<obj::ratio>(ratio_ptr) };
        CHECK_EQ(ratio->to_string(), "3/4");
      }
      SUBCASE("Hashing")
      {
        auto const ratio1{ expect_object<obj::ratio>(obj::ratio::create(2, 3)) };
        auto const ratio2{ expect_object<obj::ratio>(obj::ratio::create(2, 3)) };
        CHECK_EQ(ratio1->to_hash(), ratio2->to_hash());
      }
    }

    TEST_CASE("Ratio interaction with other data types")
    {
      obj::ratio_data const ratio{ 3, 4 };

      SUBCASE("Addition with native integer")
      {
        native_integer const i{ 2ll };
        auto const result{ *(ratio + i) };
        auto const result2{ *(i + ratio) };
        CHECK_EQ(result.data.numerator, 11);
        CHECK_EQ(result.data.denominator, 4);
        CHECK_EQ(result2.data.numerator, 11);
        CHECK_EQ(result2.data.denominator, 4);
      }
      SUBCASE("Addition with native real")
      {
        native_real const r{ 0.5 };
        auto const result{ ratio + r };
        auto const result2{ r + ratio };
        CHECK_EQ(result, doctest::Approx(1.25));
        CHECK_EQ(result2, doctest::Approx(1.25));
      }

      SUBCASE("Subtraction with boxed int")
      {
        auto const i{ make_box(1) };
        auto const result{ ratio - i };
        auto const result2{ i - ratio };
        CHECK_EQ(result->data.numerator, -1);
        CHECK_EQ(result->data.denominator, 4);
        CHECK_EQ(result2->data.numerator, 1);
        CHECK_EQ(result2->data.denominator, 4);
      }

      SUBCASE("Subtraction with boxed real")
      {
        auto const r{ make_box(0.25) };
        auto const result{ ratio - r };
        auto const result2{ r - ratio };
        CHECK_EQ(result, doctest::Approx(0.5));
        CHECK_EQ(result2, doctest::Approx(-0.5));
      }

      SUBCASE("Multiplication with boxed int")
      {
        auto const i{ make_box(3) };
        auto const result{ *expect_object<obj::ratio>(ratio * i) };
        auto const result2{ *expect_object<obj::ratio>(i * ratio) };
        CHECK_EQ(result.data.numerator, 9);
        CHECK_EQ(result.data.denominator, 4);
        CHECK_EQ(result2.data.numerator, 9);
        CHECK_EQ(result2.data.denominator, 4);
      }

      SUBCASE("Multiplication with native real")
      {
        native_real const r{ 0.5 };
        auto const result{ ratio * r };
        auto const result2{ r * ratio };
        CHECK_EQ(result, doctest::Approx(0.375));
        CHECK_EQ(result2, doctest::Approx(0.375));
      }

      SUBCASE("Division with native integer")
      {
        native_integer const i{ 2ll };
        auto const result{ ratio / i };
        auto const result2{ expect_object<obj::ratio>(i / ratio) };
        CHECK_EQ(result->data.numerator, 3);
        CHECK_EQ(result->data.denominator, 8);
        CHECK_EQ(result2->data.numerator, 8);
        CHECK_EQ(result2->data.denominator, 3);
      }

      SUBCASE("Division with native real")
      {
        native_real const r{ 0.5 };
        auto const result{ ratio / r };
        auto const result2{ r / ratio };
        CHECK_EQ(result, doctest::Approx(1.5));
        CHECK_EQ(result2, doctest::Approx(1 / 1.5));
      }

      SUBCASE("Comparison with boxed integer")
      {
        auto const i{ make_box(1ll) };
        CHECK_LT(ratio, i);
        CHECK_NE(ratio, i);
        CHECK_GT(i, ratio);
        CHECK_NE(i, ratio);
      }

      SUBCASE("Comparison with native int")
      {
        native_integer const i{ 1ll };
        CHECK_LT(ratio, i);
        CHECK_NE(ratio, i);
        CHECK_GT(i, ratio);
        CHECK_NE(i, ratio);
      }

      SUBCASE("Comparison with native real")
      {
        native_real const r{ 0.75 };
        CHECK_EQ(ratio, r);
        CHECK_EQ(r, ratio);
      }
    }

    TEST_CASE("Ratio mixed arithmetic and comparisons")
    {
      obj::ratio_data const ratio{ 5, 8 };
      native_integer const i{ 3ll };
      native_real const r{ 0.25 };

      SUBCASE("Complex arithmetic chain")
      {
        auto const result{ (ratio + i)->data - r };
        CHECK_EQ(result, doctest::Approx(3.375));
      }

      SUBCASE("Mixed comparison")
      {
        auto const real_ptr{ make_box(1.0) };
        auto const int_ptr{ make_box(1ll) };

        CHECK_LT(ratio, real_ptr);
        CHECK_LT(ratio, int_ptr);
        CHECK_GT(real_ptr, ratio);
        CHECK_GT(int_ptr, ratio);
      }
    }

    TEST_CASE("Edge case interactions")
    {
      SUBCASE("Divided by zero")
      {
        auto const result{ obj::ratio_data(1, 2) / 0.0 };
        CHECK(std::isinf(result));
        CHECK_GT(result, 0);
        CHECK_EQ(result, std::numeric_limits<double>::infinity());

        auto const neg_result{ expect_object<obj::ratio>(-1ll * obj::ratio_data(1, 2))->data
                               / 0.0 };
        CHECK(std::isinf(neg_result));
        CHECK_LT(neg_result, 0);
        CHECK_EQ(neg_result, -std::numeric_limits<double>::infinity());

        CHECK_THROWS_AS((obj::ratio_data(1, 2) / 0ll), std::invalid_argument);
        CHECK_THROWS_AS((obj::ratio_data(1, 2) / obj::ratio_data(0, 1)), std::invalid_argument);
      }

      SUBCASE("Ratio multiplied by negative integer")
      {
        auto const result{ expect_object<obj::ratio>(obj::ratio_data(2, 3) * -4ll) };
        CHECK_EQ(result->data.numerator, -8);
        CHECK_EQ(result->data.denominator, 3);
      }
    }

    TEST_CASE("Constructor")
    {
      auto const a{ expect_object<obj::ratio>(obj::ratio::create(3, 4)) };
      CHECK_EQ(a->data.numerator, 3);
      CHECK_EQ(a->data.denominator, 4);
    }

    TEST_CASE("to_real")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      CHECK_EQ(a->to_real(), 3.0 / 4.0);
    }

    TEST_CASE("to_integer")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(7, 4)) };
      CHECK_EQ(a->to_integer(), 1);
    }

    TEST_CASE("to_string")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      CHECK_EQ(a->to_string(), "3/4");
    }

    TEST_CASE("compare_less_than")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      auto const b{ make_box<obj::ratio>(obj::ratio_data(5, 4)) };
      CHECK_LT(a->compare(*b), 0);
    }

    TEST_CASE("compare_greater_than")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(5, 4)) };
      auto const b{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      CHECK_GT(a->compare(*b), 0);
    }

    TEST_CASE("compare_equal")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      auto const b{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      CHECK_EQ(a->compare(*b), 0);
    }

    TEST_CASE("is_zero")
    {
      CHECK(is_zero(make_box<obj::ratio>(obj::ratio_data(0, 1))));
    }

    TEST_CASE("is_positive")
    {
      CHECK(is_pos(make_box<obj::ratio>(obj::ratio_data(3, 4))));
    }

    TEST_CASE("is_negative")
    {
      CHECK(is_neg(make_box<obj::ratio>(obj::ratio_data(-3, 4))));
    }

    TEST_CASE("is_equivalent")
    {
      CHECK(is_equiv(make_box<obj::ratio>(obj::ratio_data(3, 4)),
                     make_box<obj::ratio>(obj::ratio_data(6, 8))));
    }

    TEST_CASE("increment")
    {
      auto const result{ expect_object<obj::ratio>(
        inc(make_box<obj::ratio>(obj::ratio_data(3, 4)))) };
      CHECK_EQ(result->data.numerator, 7);
      CHECK_EQ(result->data.denominator, 4);
    }

    TEST_CASE("decrement")
    {
      auto const result{ expect_object<obj::ratio>(
        dec(make_box<obj::ratio>(obj::ratio_data(3, 4)))) };
      CHECK_EQ(result->data.numerator, -1);
      CHECK_EQ(result->data.denominator, 4);
    }

    TEST_CASE("abs")
    {
      auto const result{ expect_object<obj::ratio>(
        abs(make_box<obj::ratio>(obj::ratio_data(-3, 4)))) };
      CHECK_EQ(result->data.numerator, 3);
      CHECK_EQ(result->data.denominator, 4);
    }

    TEST_CASE("sqrt")
    {
      auto const result{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      CHECK_EQ(sqrt(result), doctest::Approx(std::sqrt(3.0 / 4.0)));
    }

    TEST_CASE("pow")
    {
      auto const a{ make_box<obj::ratio>(obj::ratio_data(3, 4)) };
      CHECK_EQ(pow(a, a), doctest::Approx(std::pow(3.0 / 4.0, 3.0 / 4.0)));
    }
  }
}
