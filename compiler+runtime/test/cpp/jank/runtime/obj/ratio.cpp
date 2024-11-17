#include "jank/runtime/obj/ratio.hpp"
#include <doctest/doctest.h>
#include <stdexcept>

namespace jank::runtime
{
  TEST_SUITE("ratio_data")
  {
    TEST_CASE("Ratio Constructor")
    {
      SUBCASE("Valid Ratio")
      {
        obj::ratio_data ratio{ 4, 6 };
        CHECK(ratio.numerator == 2); // Simplified
        CHECK(ratio.denominator == 3);
      }
      SUBCASE("Invalid Denominator")
      {
        CHECK_THROWS_AS(obj::ratio_data(4, 0), std::invalid_argument);
      }
    }

    TEST_CASE("ToReal Conversion")
    {
      obj::ratio_data ratio{ 3, 4 };
      CHECK(ratio.to_real() == doctest::Approx(0.75));
    }

    TEST_CASE("Simplify Functionality")
    {
      obj::ratio_data ratio{ 10, -20 };
      CHECK(ratio.numerator == -1);
      CHECK(ratio.denominator == 2);
    }

    TEST_CASE("Create Ratio or Integer")
    {
      SUBCASE("Simplified to Integer")
      {
        auto ratio_ptr = obj::ratio::create(4, 2);
        CHECK(ratio_ptr->type == object_type::integer);
        REQUIRE(ratio_ptr != nullptr);
        CHECK(expect_object<obj::integer>(ratio_ptr)->data == 2);
      }
      SUBCASE("Remains as Ratio")
      {
        auto ratio_ptr = obj::ratio::create(3, 4);
        auto ratio = expect_object<obj::ratio>(ratio_ptr);
        REQUIRE(ratio != nullptr);
        CHECK(ratio->data.numerator == 3);
        CHECK(ratio->data.denominator == 4);
        CHECK(ratio->to_real() == doctest::Approx(0.75));
      }
    }

    TEST_CASE("Arithmetic Operations")
    {
      obj::ratio_data a{ 1, 2 }, b{ 1, 3 };

      SUBCASE("Addition")
      {
        auto result = *expect_object<obj::ratio>(a + b);
        CHECK(result.data.numerator == 5);
        CHECK(result.data.denominator == 6);
      }
      SUBCASE("Subtraction")
      {
        auto result = *expect_object<obj::ratio>(a - b);
        CHECK(result.data.numerator == 1);
        CHECK(result.data.denominator == 6);
      }
      SUBCASE("Multiplication")
      {
        auto result = *expect_object<obj::ratio>(a * b);
        CHECK(result.data.numerator == 1);
        CHECK(result.data.denominator == 6);
      }

      SUBCASE("Division")
      {
        auto result = *expect_object<obj::ratio>(a / b);
        CHECK(result.data.numerator == 3);
        CHECK(result.data.denominator == 2);
      }
    }

    TEST_CASE("Comparison Operations")
    {
      obj::ratio_data a{ 1, 2 }, b{ 2, 4 }, c{ 1, 3 };

      SUBCASE("Equality")
      {
        CHECK(a == b);
        CHECK_FALSE(a == c);
      }
      SUBCASE("Less Than")
      {
        CHECK(c < a);
        CHECK_FALSE(a < b);
      }
      SUBCASE("Greater Than")
      {
        CHECK(a > c);
        CHECK_FALSE(c > a);
      }
    }

    TEST_CASE("Edge Cases and Utility Functions")
    {
      obj::ratio_data ratio{ 6, -9 };

      SUBCASE("Simplify on Negative Denominator")
      {
        CHECK(ratio.numerator == -2);
        CHECK(ratio.denominator == 3);
      }
      SUBCASE("ToString")
      {
        auto ratio_ptr = obj::ratio::create(3, 4);
        auto ratio = expect_object<obj::ratio>(ratio_ptr);
        CHECK(ratio->to_string() == "3/4");
      }
      SUBCASE("Hashing")
      {
        auto ratio1 = expect_object<obj::ratio>(obj::ratio::create(2, 3));
        auto ratio2 = expect_object<obj::ratio>(obj::ratio::create(2, 3));
        CHECK(ratio1->to_hash() == ratio2->to_hash());
      }
    }

    TEST_CASE("Ratio Interaction with Other Data Types")
    {
      obj::ratio_data ratio{ 3, 4 };

      SUBCASE("Addition with Native Integer")
      {
        auto native_int{ make_box(2LL) };
        auto result = *(ratio + native_int);
        auto result2 = *(native_int + ratio);
        CHECK(result.data.numerator == 11);
        CHECK(result.data.denominator == 4);
        CHECK(result2.data.numerator == 11);
        CHECK(result2.data.denominator == 4);
      }
      SUBCASE("Addition with Native Real")
      {
        auto native_real{ make_box(0.5) };
        auto result = ratio + native_real;
        auto result2 = native_real + ratio;
        CHECK(result == doctest::Approx(1.25));
        CHECK(result2 == doctest::Approx(1.25));
      }
      SUBCASE("Subtraction with Integer Pointer")
      {
        auto int_ptr = make_box<obj::integer>(1);
        auto result = ratio - int_ptr;
        auto result2 = int_ptr - ratio;
        CHECK(result->data.numerator == -1);
        CHECK(result->data.denominator == 4);
        CHECK(result2->data.numerator == 1);
        CHECK(result2->data.denominator == 4);
      }

      SUBCASE("Subtraction with Native Real")
      {
        auto native_real{ make_box(0.25) };
        auto result = ratio - native_real;
        auto result2 = native_real - ratio;
        CHECK(result == doctest::Approx(0.5));
        CHECK(result2 == doctest::Approx(-0.5));
      }

      SUBCASE("Multiplication with Integer Pointer")
      {
        auto int_ptr{ make_box(3) };
        auto result = *expect_object<obj::ratio>(ratio * int_ptr);
        auto result2 = *expect_object<obj::ratio>(int_ptr * ratio);
        CHECK(result.data.numerator == 9);
        CHECK(result.data.denominator == 4);
        CHECK(result2.data.numerator == 9);
        CHECK(result2.data.denominator == 4);
      }

      SUBCASE("Multiplication with Native Real")
      {
        auto native_real{ 0.5L };
        auto result = ratio * native_real;
        auto result2 = native_real * ratio;
        CHECK(result == doctest::Approx(0.375));
        CHECK(result2 == doctest::Approx(0.375));
      }

      SUBCASE("Division with Native Integer")
      {
        auto native_int{ 2LL };
        auto result = ratio / native_int;
        auto result2 = expect_object<obj::ratio>(native_int / ratio);
        CHECK(result->data.numerator == 3);
        CHECK(result->data.denominator == 8);
        CHECK(result2->data.numerator == 8);
        CHECK(result2->data.denominator == 3);
      }

      SUBCASE("Division with Native Real")
      {
        auto native_real{ 0.5L };
        auto result = ratio / native_real;
        auto result2 = native_real / ratio;
        CHECK(result == doctest::Approx(1.5));
        CHECK(result2 == doctest::Approx(1 / 1.5));
      }

      SUBCASE("Comparison with Integer Pointer")
      {
        auto int_ptr{ make_box(1LL) };
        CHECK(ratio < int_ptr);
        CHECK(ratio != int_ptr);
        CHECK(int_ptr > ratio);
        CHECK(int_ptr != ratio);
      }

      SUBCASE("Comparison with Native Integer")
      {
        auto native_int{ 1LL };
        CHECK(ratio < native_int);
        CHECK(ratio != native_int);
        CHECK(native_int > ratio);
        CHECK(native_int != ratio);
      }

      SUBCASE("Comparison with Native Real")
      {
        auto native_real{ 0.75L };
        CHECK(ratio == native_real);
        CHECK(native_real == ratio);
      }
    }

    TEST_CASE("Ratio Mixed Arithmetic and Comparisons")
    {
      obj::ratio_data ratio{ 5, 8 };
      auto native_int{ 3LL };
      auto native_real{ 0.25L };

      SUBCASE("Complex Arithmetic Chain")
      {
        auto result = (ratio + native_int)->data - native_real;
        CHECK(result == doctest::Approx(3.375));
      }

      SUBCASE("Mixed Comparison")
      {
        auto real_ptr{ make_box(1.0L) };
        auto int_ptr{ make_box(1LL) };

        CHECK(ratio < real_ptr);
        CHECK(ratio < int_ptr);
        CHECK(real_ptr > ratio);
        CHECK(int_ptr > ratio);
      }
    }

    TEST_CASE("Edge Case Interactions")
    {
      SUBCASE("Ratio Divided by Zero")
      {
        auto result = obj::ratio_data(1, 2) / 0.0L;
        CHECK(std::isinf(result));
        CHECK(result > 0);
        CHECK(result == std::numeric_limits<double>::infinity());

        auto neg_result = expect_object<obj::ratio>(-1LL * obj::ratio_data(1, 2))->data / 0.0L;
        CHECK(std::isinf(neg_result));
        CHECK(neg_result < 0);
        CHECK(neg_result == -std::numeric_limits<double>::infinity());

        CHECK_THROWS_AS((obj::ratio_data(1, 2) / 0LL), std::invalid_argument);
        CHECK_THROWS_AS((obj::ratio_data(1, 2) / obj::ratio_data(0, 1)), std::invalid_argument);
      }

      SUBCASE("Ratio Multiplied by Negative Integer")
      {
        auto result = expect_object<obj::ratio>(obj::ratio_data(2, 3) * -4LL);
        CHECK(result->data.numerator == -8);
        CHECK(result->data.denominator == 3);
      }
    }
  }

  TEST_SUITE("ratio")
  {
    obj::ratio_ptr make_ptr(native_integer num, native_integer denom)
    {
      auto r(make_box<obj::ratio>());
      r->data.numerator = num;
      r->data.denominator = denom;
      return r;
    }

    TEST_CASE("constructor")
    {
      auto a = expect_object<obj::ratio>(obj::ratio::create(3, 4));
      CHECK(a->data.numerator == 3);
      CHECK(a->data.denominator == 4);
    }

    TEST_CASE("to_real")
    {
      auto a = make_ptr(3, 4);
      CHECK(a->to_real() == 3.0 / 4.0);
    }

    TEST_CASE("to_integer")
    {
      auto a = make_ptr(7, 4);
      CHECK(a->to_integer() == 1);
    }

    TEST_CASE("to_string")
    {
      auto a = make_ptr(3, 4);
      CHECK(a->to_string() == "3/4");
    }


    TEST_CASE("compare_less_than")
    {
      auto a = make_ptr(3, 4);
      auto b = make_ptr(5, 4);
      CHECK(a->compare(*b) < 0);
    }

    TEST_CASE("compare_greater_than")
    {
      auto a = make_ptr(5, 4);
      auto b = make_ptr(3, 4);
      CHECK(a->compare(*b) > 0);
    }

    TEST_CASE("compare_equal")
    {
      auto a = make_ptr(3, 4);
      auto b = make_ptr(3, 4);
      CHECK(a->compare(*b) == 0);
    }

    TEST_CASE("is_zero")
    {
      CHECK(is_zero(make_ptr(0, 1)));
    }

    TEST_CASE("is_positive")
    {
      obj::ratio;
      CHECK(is_pos(make_ptr(3, 4)));
    }

    TEST_CASE("is_negative")
    {
      CHECK(is_neg(make_ptr(-3, 4)));
    }

    TEST_CASE("is_equivalent")
    {
      CHECK(is_equiv(make_ptr(3, 4), make_ptr(6, 8)));
    }

    TEST_CASE("increment")
    {
      auto result = expect_object<obj::ratio>(inc(make_ptr(3, 4)));
      CHECK(result->data.numerator == 7);
      CHECK(result->data.denominator == 4);
    }

    TEST_CASE("decrement")
    {
      auto result = expect_object<obj::ratio>(dec(make_ptr(3, 4)));
      CHECK(result->data.numerator == -1);
      CHECK(result->data.denominator == 4);
    }

    TEST_CASE("abs")
    {
      auto result = expect_object<obj::ratio>(abs(make_ptr(-3, 4)));
      CHECK(result->data.numerator == 3);
      CHECK(result->data.denominator == 4);
    }

    TEST_CASE("sqrt")
    {
      auto result = make_ptr(3, 4);
      CHECK(sqrt(result) == doctest::Approx(std::sqrt(3.0 / 4.0)));
    }

    TEST_CASE("pow")
    {
      auto a = make_ptr(3, 4);
      CHECK(pow(a, a) == doctest::Approx(std::pow(3.0 / 4.0, 3.0 / 4.0)));
    }
  }
}
