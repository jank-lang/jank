#include <stdexcept>
#include <jank/runtime/obj/ratio.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("ratio")
  {
    TEST_CASE("Ratio Constructor")
    {
      SUBCASE("Valid Ratio")
      {
        obj::ratio_data ratio{ 4, 6 };
        CHECK_EQ(ratio.numerator, 2); // Simplified
        CHECK_EQ(ratio.denominator, 3);
      }
      SUBCASE("Invalid Denominator")
      {
        CHECK_THROWS_AS(obj::ratio_data(4, 0), std::invalid_argument);
      }
    }

    TEST_CASE("ToReal Conversion")
    {
      obj::ratio_data ratio{ 3, 4 };
      CHECK_EQ(ratio.to_real(), doctest::Approx(0.75));
    }

    TEST_CASE("Simplify Functionality")
    {
      obj::ratio_data ratio{ 10, -20 };
      CHECK_EQ(ratio.numerator, -1);
      CHECK_EQ(ratio.denominator, 2);
    }

    TEST_CASE("Create Ratio or Integer")
    {
      SUBCASE("Simplified to Integer")
      {
        auto const ratio_ptr{ obj::ratio::create(4, 2) };
        CHECK_EQ(ratio_ptr->type, object_type::integer);
        REQUIRE(ratio_ptr != nullptr);
        CHECK_EQ(expect_object<obj::integer>(ratio_ptr)->data, 2);
      }
      SUBCASE("Remains as Ratio")
      {
        auto const ratio_ptr{ obj::ratio::create(3, 4) };
        auto const ratio{ expect_object<obj::ratio>(ratio_ptr) };
        REQUIRE(ratio != nullptr);
        CHECK_EQ(ratio->data.numerator, 3);
        CHECK_EQ(ratio->data.denominator, 4);
        CHECK_EQ(ratio->to_real(), doctest::Approx(0.75));
      }
    }

    TEST_CASE("Arithmetic Operations")
    {
      obj::ratio_data a{ 1, 2 }, b{ 1, 3 };

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

    TEST_CASE("Comparison Operations")
    {
      obj::ratio_data a{ 1, 2 }, b{ 2, 4 }, c{ 1, 3 };

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

    TEST_CASE("Edge Cases and Utility Functions")
    {
      obj::ratio_data ratio{ 6, -9 };

      SUBCASE("Simplify on Negative Denominator")
      {
        CHECK_EQ(ratio.numerator, -2);
        CHECK_EQ(ratio.denominator, 3);
      }
      SUBCASE("ToString")
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

    TEST_CASE("Ratio Interaction with Other Data Types")
    {
      obj::ratio_data ratio{ 3, 4 };

      SUBCASE("Addition with Native Integer")
      {
        auto const native_int{ make_box(2LL) };
        auto const result{ *(ratio + native_int) };
        auto const result2{ *(native_int + ratio) };
        CHECK_EQ(result.data.numerator, 11);
        CHECK_EQ(result.data.denominator, 4);
        CHECK_EQ(result2.data.numerator, 11);
        CHECK_EQ(result2.data.denominator, 4);
      }
      SUBCASE("Addition with Native Real")
      {
        auto const native_real{ make_box(0.5) };
        auto const result{ ratio + native_real };
        auto const result2{ native_real + ratio };
        CHECK_EQ(result, doctest::Approx(1.25));
        CHECK_EQ(result2, doctest::Approx(1.25));
      }
      SUBCASE("Subtraction with Integer Pointer")
      {
        auto const int_ptr{ make_box<obj::integer>(1) };
        auto const result{ ratio - int_ptr };
        auto const result2{ int_ptr - ratio };
        CHECK_EQ(result->data.numerator, -1);
        CHECK_EQ(result->data.denominator, 4);
        CHECK_EQ(result2->data.numerator, 1);
        CHECK_EQ(result2->data.denominator, 4);
      }

      SUBCASE("Subtraction with Native Real")
      {
        auto const native_real{ make_box(0.25) };
        auto const result{ ratio - native_real };
        auto const result2{ native_real - ratio };
        CHECK_EQ(result, doctest::Approx(0.5));
        CHECK_EQ(result2, doctest::Approx(-0.5));
      }

      SUBCASE("Multiplication with Integer Pointer")
      {
        auto const int_ptr{ make_box(3) };
        auto const result{ *expect_object<obj::ratio>(ratio * int_ptr) };
        auto const result2{ *expect_object<obj::ratio>(int_ptr * ratio) };
        CHECK_EQ(result.data.numerator, 9);
        CHECK_EQ(result.data.denominator, 4);
        CHECK_EQ(result2.data.numerator, 9);
        CHECK_EQ(result2.data.denominator, 4);
      }

      SUBCASE("Multiplication with Native Real")
      {
        auto const native_real{ 0.5L };
        auto const result{ ratio * native_real };
        auto const result2{ native_real * ratio };
        CHECK_EQ(result, doctest::Approx(0.375));
        CHECK_EQ(result2, doctest::Approx(0.375));
      }

      SUBCASE("Division with Native Integer")
      {
        auto const native_int{ 2LL };
        auto const result{ ratio / native_int };
        auto const result2{ expect_object<obj::ratio>(native_int / ratio) };
        CHECK_EQ(result->data.numerator, 3);
        CHECK_EQ(result->data.denominator, 8);
        CHECK_EQ(result2->data.numerator, 8);
        CHECK_EQ(result2->data.denominator, 3);
      }

      SUBCASE("Division with Native Real")
      {
        auto const native_real{ 0.5L };
        auto const result{ ratio / native_real };
        auto const result2{ native_real / ratio };
        CHECK_EQ(result, doctest::Approx(1.5));
        CHECK_EQ(result2, doctest::Approx(1 / 1.5));
      }

      SUBCASE("Comparison with Integer Pointer")
      {
        auto const int_ptr{ make_box(1LL) };
        CHECK_LT(ratio, int_ptr);
        CHECK_NE(ratio, int_ptr);
        CHECK_GT(int_ptr, ratio);
        CHECK_NE(int_ptr, ratio);
      }

      SUBCASE("Comparison with Native Integer")
      {
        auto const native_int{ 1LL };
        CHECK_LT(ratio, native_int);
        CHECK_NE(ratio, native_int);
        CHECK_GT(native_int, ratio);
        CHECK_NE(native_int, ratio);
      }

      SUBCASE("Comparison with Native Real")
      {
        auto const native_real{ 0.75L };
        CHECK_EQ(ratio, native_real);
        CHECK_EQ(native_real, ratio);
      }
    }

    TEST_CASE("Ratio Mixed Arithmetic and Comparisons")
    {
      obj::ratio_data ratio{ 5, 8 };
      auto const native_int{ 3LL };
      auto const native_real{ 0.25L };

      SUBCASE("Complex Arithmetic Chain")
      {
        auto const result{ (ratio + native_int)->data - native_real };
        CHECK_EQ(result, doctest::Approx(3.375));
      }

      SUBCASE("Mixed Comparison")
      {
        auto const real_ptr{ make_box(1.0L) };
        auto const int_ptr{ make_box(1LL) };

        CHECK_LT(ratio, real_ptr);
        CHECK_LT(ratio, int_ptr);
        CHECK_GT(real_ptr, ratio);
        CHECK_GT(int_ptr, ratio);
      }
    }

    TEST_CASE("Edge Case Interactions")
    {
      SUBCASE("Ratio Divided by Zero")
      {
        auto const result{ obj::ratio_data(1, 2) / 0.0L };
        CHECK(std::isinf(result));
        CHECK_GT(result, 0);
        CHECK_EQ(result, std::numeric_limits<double>::infinity());

        auto const neg_result{ expect_object<obj::ratio>(-1LL * obj::ratio_data(1, 2))->data
                               / 0.0L };
        CHECK(std::isinf(neg_result));
        CHECK_LT(neg_result, 0);
        CHECK_EQ(neg_result, -std::numeric_limits<double>::infinity());

        CHECK_THROWS_AS((obj::ratio_data(1, 2) / 0LL), std::invalid_argument);
        CHECK_THROWS_AS((obj::ratio_data(1, 2) / obj::ratio_data(0, 1)), std::invalid_argument);
      }

      SUBCASE("Ratio Multiplied by Negative Integer")
      {
        auto const result{ expect_object<obj::ratio>(obj::ratio_data(2, 3) * -4LL) };
        CHECK_EQ(result->data.numerator, -8);
        CHECK_EQ(result->data.denominator, 3);
      }
    }
  }

  TEST_CASE("constructor")
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
    obj::ratio;
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
