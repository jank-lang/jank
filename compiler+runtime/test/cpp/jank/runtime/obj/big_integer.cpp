#include <limits>
#include <stdexcept>
#include <string>
#include <cmath>

#include <jank/runtime/obj/big_integer.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/string_builder.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime::obj
{
  static native_big_integer nbi(long long const val)
  {
    return native_big_integer{ val };
  }

  static native_big_integer nbi(char const * const s)
  {
    return native_big_integer{ s };
  }

  TEST_SUITE("big_integer")
  {
    TEST_CASE("equal")
    {
      auto const bi1{ make_box<big_integer>(nbi("123456789012345")) };
      auto const bi2{ make_box<big_integer>(nbi("123456789012345")) };
      auto const bi3{ make_box<big_integer>(nbi("-123456789012345")) };
      auto const bi_zero{ make_box<big_integer>(0) };
      auto const i1{ make_box<integer>(12345LL) };
      auto const i2{ make_box<integer>(12345LL) };
      auto const i3{ make_box<integer>(-12345LL) };
      auto const r1{ make_box<real>(12345.0) };
      auto const r2{ make_box<real>(12345.00000000001) };
      auto const r3{ make_box<real>(12345.0000000000000001) };
      auto const r_large_exact{ make_box<real>(123456789012345.0) };
      auto const r_large_approx{ make_box<real>(123456789012345.00001) };
      auto const ratio1{ make_box<obj::ratio>(obj::ratio_data(1, 2)) };

      SUBCASE("big_integer vs big_integer")
      {
        CHECK(bi1->equal(*bi2.erase()));
        CHECK_FALSE(bi1->equal(*bi3.erase()));
        CHECK_FALSE(bi1->equal(*bi_zero.erase()));
      }

      SUBCASE("big_integer vs integer")
      {
        big_integer const bi_i1(12345LL);
        big_integer const bi_i3(-12345LL);
        CHECK(bi_i1.equal(*i1.erase()));
        CHECK(bi_i1.equal(*i2.erase()));
        CHECK_FALSE(bi_i1.equal(*i3.erase()));
        CHECK(bi_i3.equal(*i3.erase()));
        CHECK_FALSE(bi_i3.equal(*i1.erase()));
      }

      SUBCASE("big_integer vs real (epsilon comparison)")
      {
        big_integer const bi_r1(12345LL);
        big_integer const bi_r_large(nbi("123456789012345"));

        CHECK(bi_r1.equal(*r1.erase()));
        CHECK_FALSE(bi_r1.equal(*r2.erase()));
        CHECK(bi_r1.equal(*r3.erase()));


        CHECK(bi_r_large.equal(*r_large_exact.erase()));
        CHECK(bi_r_large.equal(*r_large_approx.erase()));

        /* Numbers that cause overflow during conversion in `equal`. */
        /* Create a BI larger than double can represent. */
        native_big_integer const huge_val{ nbi("1") << 1024 };
        big_integer const bi_huge_pos(huge_val);
        big_integer const bi_huge_neg(-huge_val);
        auto r_inf_pos{ make_box<real>(std::numeric_limits<f64>::infinity()) };
        auto r_inf_neg{ make_box<real>(-std::numeric_limits<f64>::infinity()) };

        /* Conversion works, but comparison should use epsilon logic which fails for inf. */
        CHECK_FALSE(bi_huge_pos.equal(*r_inf_pos.erase()));
        CHECK_FALSE(bi_huge_neg.equal(*r_inf_neg.erase()));
        CHECK_FALSE(bi_huge_pos.equal(*r1.erase()));
      }

      SUBCASE("big_integer vs other type")
      {
        CHECK_FALSE(bi1->equal(*ratio1.erase()));
      }
    }

    TEST_CASE("String conversions")
    {
      big_integer const bi_pos(nbi("1234567890987654321"));
      big_integer const bi_neg(nbi("-9876543210123456789"));
      big_integer const bi_zero(0);

      SUBCASE("to_string()")
      {
        CHECK_EQ(bi_pos.to_string(), "1234567890987654321N");
        CHECK_EQ(bi_neg.to_string(), "-9876543210123456789N");
        CHECK_EQ(bi_zero.to_string(), "0N");
      }

      SUBCASE("to_code_string()")
      {
        CHECK_EQ(bi_pos.to_code_string(), "1234567890987654321N");
        CHECK_EQ(bi_neg.to_code_string(), "-9876543210123456789N");
        CHECK_EQ(bi_zero.to_code_string(), "0N");
      }

      SUBCASE("to_string(string_builder)")
      {
        util::string_builder sb1;
        bi_pos.to_string(sb1);
        CHECK_EQ(sb1.view(), "1234567890987654321N");

        util::string_builder sb2;
        bi_neg.to_string(sb2);
        CHECK_EQ(sb2.view(), "-9876543210123456789N");

        util::string_builder sb3;
        bi_zero.to_string(sb3);
        CHECK_EQ(sb3.view(), "0N");
      }
    }

    TEST_CASE("Hashing")
    {
      big_integer const bi1(nbi("12345678901234567890"));
      big_integer const bi2(nbi("12345678901234567890"));
      big_integer const bi3(nbi("-12345678901234567890"));
      big_integer const bi4(0);
      big_integer const bi5(-1);

      SUBCASE("Equality implies same hash")
      {
        CHECK_EQ(bi1.to_hash(), bi2.to_hash());
      }

      SUBCASE("Inequality implies different hash (highly likely)")
      {
        CHECK_NE(bi1.to_hash(), bi3.to_hash());
        CHECK_NE(bi1.to_hash(), bi4.to_hash());
        CHECK_NE(bi3.to_hash(), bi4.to_hash());
        CHECK_NE(bi4.to_hash(), bi5.to_hash());
      }

      SUBCASE("Static hash function")
      {
        CHECK_EQ(big_integer::to_hash(bi1.data), bi1.to_hash());
        CHECK_EQ(big_integer::to_hash(bi3.data), bi3.to_hash());
        CHECK_EQ(big_integer::to_hash(bi4.data), bi4.to_hash());
        CHECK_EQ(big_integer::to_hash(nbi(LLONG_MAX)), make_box<big_integer>(LLONG_MAX)->to_hash());
        CHECK_EQ(big_integer::to_hash(nbi(LLONG_MIN)), make_box<big_integer>(LLONG_MIN)->to_hash());
      }
    }

    TEST_CASE("compare method")
    {
      auto bi_10{ make_box<big_integer>(10) };
      auto bi_20{ make_box<big_integer>(20) };
      auto bi_10_neg{ make_box<big_integer>(-10) };
      auto bi_large{ make_box<big_integer>(nbi("100000000000000000000")) };

      auto i_15{ make_box<integer>(15) };
      auto i_10{ make_box<integer>(10) };
      auto i_10_neg{ make_box<integer>(-10) };

      auto r_10{ make_box<real>(10.0) };
      auto r_10_5{ make_box<real>(10.5) };
      auto r_neg_10{ make_box<real>(-10.0) };
      auto r_neg_9_5{ make_box<real>(-9.5) };

      auto ratio_half{ make_box<obj::ratio>(obj::ratio_data(1, 2)) };
      auto bool_true{ make_box<obj::boolean>(true) };

      SUBCASE("big_integer vs big_integer")
      {
        CHECK_LT(bi_10->compare(*bi_20.erase()), 0);
        CHECK_GT(bi_20->compare(*bi_10.erase()), 0);
        CHECK_EQ(bi_10->compare(*make_box<big_integer>(10).erase()), 0);
        CHECK_GT(bi_10->compare(*bi_10_neg.erase()), 0);
        CHECK_LT(bi_10_neg->compare(*bi_10.erase()), 0);
        CHECK_GT(bi_large->compare(*bi_20.erase()), 0);
      }

      SUBCASE("big_integer vs integer")
      {
        CHECK_LT(bi_10->compare(*i_15.erase()), 0);
        CHECK_EQ(bi_10->compare(*i_10.erase()), 0);
        CHECK_GT(bi_10->compare(*i_10_neg.erase()), 0);
        CHECK_GT(bi_large->compare(*make_box<integer>(LLONG_MAX).erase()), 0);
      }

      SUBCASE("big_integer vs real")
      {
        CHECK_LT(bi_10->compare(*r_10_5.erase()), 0);
        CHECK_EQ(bi_10->compare(*r_10.erase()), 0);
        CHECK_GT(bi_10->compare(*r_neg_10.erase()), 0);
        CHECK_LT(bi_10_neg->compare(*r_neg_9_5.erase()), 0);
        CHECK_GT(bi_large->compare(*make_box<real>(static_cast<f64>(LLONG_MAX)).erase()), 0);
      }

      SUBCASE("big_integer vs ratio")
      {
        CHECK_GT(bi_10->compare(*ratio_half.erase()), 0);
        CHECK_EQ(make_box<big_integer>(0)->compare(*ratio_half.erase()), -1);
      }

      SUBCASE("big_integer vs incompatible")
      {
        CHECK_THROWS_AS(bi_10->compare(*bool_true.erase()), std::runtime_error);
      }
    }

    TEST_CASE("Static gcd")
    {
      CHECK_EQ(big_integer::gcd(nbi(12), nbi(18)), 6);
      CHECK_EQ(big_integer::gcd(nbi(-12), nbi(18)), 6);
      CHECK_EQ(big_integer::gcd(nbi("123456789012"), nbi("987654321098")), 2);
    }

    TEST_CASE("to_integer / to_i64")
    {
      SUBCASE("Within i64 range")
      {
        CHECK_EQ(big_integer(123).to_integer(), 123LL);
        CHECK_EQ(big_integer(0).to_integer(), 0LL);
        CHECK_EQ(big_integer(-456).to_integer(), -456LL);
        CHECK_EQ(big_integer(LLONG_MAX).to_integer(), LLONG_MAX);
        CHECK_EQ(big_integer(LLONG_MIN).to_integer(), LLONG_MIN);
      }

      SUBCASE("Outside i64 range (throws)")
      {
        native_big_integer max_plus_1{ nbi(LLONG_MAX) };
        ++max_plus_1;
        native_big_integer min_minus_1{ nbi(LLONG_MIN) };
        --min_minus_1;

        CHECK_THROWS_AS(big_integer(max_plus_1).to_integer(), std::runtime_error);
        CHECK_THROWS_AS(big_integer(min_minus_1).to_integer(), std::runtime_error);

        native_big_integer const very_large{ nbi(1) << 100 }; /* 2^100 */
        CHECK_THROWS_AS(big_integer(very_large).to_integer(), std::runtime_error);
        CHECK_THROWS_AS(big_integer(-very_large).to_integer(), std::runtime_error);
      }
    }

    TEST_CASE("to_real / to_f64")
    {
      SUBCASE("Within f64 range")
      {
        CHECK_EQ(big_integer(123).to_real(), doctest::Approx(123.0));
        CHECK_EQ(big_integer(0).to_real(), doctest::Approx(0.0));
        CHECK_EQ(big_integer(-456).to_real(), doctest::Approx(-456.0));
        /* A large number exactly representable by double. */
        native_big_integer const exact_double_val{ nbi(1) << 50 };
        CHECK_EQ(big_integer(exact_double_val).to_real(), doctest::Approx(std::pow(2.0, 50)));
      }

      SUBCASE("Outside f64 range (infinity)")
      {
        native_big_integer const huge_pos{ nbi(1) << 1024 }; /* 2^1024 > DBL_MAX */
        native_big_integer const huge_neg{ -huge_pos };
        CHECK_EQ(big_integer(huge_pos).to_real(), std::numeric_limits<f64>::infinity());
        CHECK_EQ(big_integer(huge_neg).to_real(), -std::numeric_limits<f64>::infinity());
      }
    }

    TEST_CASE("Operators (big_integer vs f64)")
    {
      using namespace jank::runtime;

      native_big_integer const bi_10{ nbi(10) };
      native_big_integer const bi_neg_5{ nbi(-5) };
      native_big_integer const bi_large{ nbi("10000000000000000") };
      constexpr f64 r_3_5{ 3.5 };
      constexpr f64 r_10{ 10.0 };
      constexpr f64 r_large{ 1.0e16 };

      SUBCASE("Addition")
      {
        CHECK_EQ(operator+(bi_10, r_3_5), doctest::Approx(13.5));
        CHECK_EQ(operator+(r_3_5, bi_10), doctest::Approx(13.5));
        CHECK_EQ(operator+(bi_neg_5, r_3_5), doctest::Approx(-1.5));
        CHECK_EQ(operator+(r_3_5, bi_neg_5), doctest::Approx(-1.5));
      }
      SUBCASE("Subtraction")
      {
        CHECK_EQ(operator-(bi_10, r_3_5), doctest::Approx(6.5));
        CHECK_EQ(operator-(r_3_5, bi_10), doctest::Approx(-6.5));
        CHECK_EQ(operator-(bi_neg_5, r_3_5), doctest::Approx(-8.5));
        CHECK_EQ(operator-(r_3_5, bi_neg_5), doctest::Approx(8.5));
      }
      SUBCASE("Multiplication")
      {
        CHECK_EQ(operator*(bi_10, r_3_5), doctest::Approx(35.0));
        CHECK_EQ(operator*(r_3_5, bi_10), doctest::Approx(35.0));
        CHECK_EQ(operator*(bi_neg_5, r_3_5), doctest::Approx(-17.5));
        CHECK_EQ(operator*(r_3_5, bi_neg_5), doctest::Approx(-17.5));
      }
      SUBCASE("Division")
      {
        constexpr f64 r_neg_2{ -2.0 };
        CHECK_EQ(operator/(bi_10, r_neg_2), doctest::Approx(-5.0));
        CHECK_EQ(operator/(r_neg_2, bi_10), doctest::Approx(-0.2));
        CHECK_EQ(operator/(bi_neg_5, r_neg_2), doctest::Approx(2.5));
        CHECK_EQ(operator/(r_neg_2, bi_neg_5), doctest::Approx(0.4));
      }
      SUBCASE("Equality (epsilon)")
      {
        constexpr f64 epsilon{ std::numeric_limits<f64>::epsilon() };
        constexpr f64 r_10_eps{ 10.0 + (epsilon / 2.0) };
        constexpr f64 r_10_10eps{ 10.0 + (epsilon * 10.0) };

        CHECK(operator==(bi_10, r_10));
        CHECK(operator==(r_10, bi_10));
        CHECK(operator==(bi_large, r_large));
        CHECK(operator==(r_large, bi_large));

        CHECK(operator==(bi_10, r_10_eps));
        CHECK(operator==(r_10_eps, bi_10));

        CHECK_FALSE(operator==(bi_10, r_10_10eps));
        CHECK_FALSE(operator==(r_10_10eps, bi_10));

        CHECK_FALSE(operator==(bi_10, r_3_5));
        CHECK_FALSE(operator==(r_3_5, bi_10));
      }
      SUBCASE("Inequality")
      {
        CHECK_FALSE(operator!=(bi_10, r_10));
        CHECK_FALSE(operator!=(r_10, bi_10));
        CHECK(operator!=(bi_10, r_3_5));
        CHECK(operator!=(r_3_5, bi_10));
      }
      SUBCASE("Less Than")
      {
        CHECK(operator<(bi_10, r_large));
        CHECK(operator<(r_3_5, bi_10));
        CHECK_FALSE(operator<(bi_10, r_3_5));
        CHECK_FALSE(operator<(bi_10, r_10));
      }
      SUBCASE("Less Than or Equal")
      {
        CHECK(operator<=(bi_10, r_large));
        CHECK(operator<=(r_3_5, bi_10));
        CHECK_FALSE(operator<=(bi_10, r_3_5));
        CHECK(operator<=(bi_10, r_10));
        CHECK(operator<=(r_10, bi_10));
      }
      SUBCASE("Greater Than")
      {
        CHECK(operator>(bi_large, r_10));
        CHECK(operator>(bi_10, r_3_5));
        CHECK_FALSE(operator>(r_3_5, bi_10));
        CHECK_FALSE(operator>(bi_10, r_10));
      }
      SUBCASE("Greater Than or Equal")
      {
        CHECK(operator>=(bi_large, r_10));
        CHECK(operator>=(bi_10, r_3_5));
        CHECK_FALSE(operator>=(r_3_5, bi_10));
        CHECK(operator>=(bi_10, r_10));
        CHECK(operator>=(r_10, bi_10));
      }
    }

    TEST_CASE("BigInteger string constructor with radix")
    {
      using namespace jank::runtime::obj;
      using boost::multiprecision::cpp_int;

      SUBCASE("Radix 2 (Binary)")
      {
        CHECK_EQ(big_integer("101010", 2, false).data, cpp_int(42));
        CHECK_EQ(big_integer("11111111", 2, false).data, cpp_int(255));
        CHECK_EQ(big_integer("100000000", 2, true).data, cpp_int(-256));
        CHECK_EQ(big_integer("0", 2, false).data, cpp_int(0));

        cpp_int const expected_large_bin{ (cpp_int(1) << 64) - 1 }; /* 2^64 - 1 */
        CHECK_EQ(
          big_integer("1111111111111111111111111111111111111111111111111111111111111111", 2, false)
            .data,
          expected_large_bin);
      }

      SUBCASE("Radix 8 (Octal)")
      {
        /* Use Boost's "0" prefix parsing for octal in expected value. */
        CHECK_EQ(big_integer("777", 8, false).data, cpp_int("0777"));
        CHECK_EQ(big_integer("12345", 8, false).data, cpp_int("012345"));
        CHECK_EQ(big_integer("10", 8, true).data, cpp_int("-010"));
        CHECK_EQ(big_integer("0", 8, false).data, cpp_int(0));
      }

      SUBCASE("Radix 16 (Hex)")
      {
        CHECK_EQ(big_integer("abcdef", 16, false).data, cpp_int("0xabcdef"));
        CHECK_EQ(big_integer("FFFFFFFF", 16, false).data, cpp_int("0xFFFFFFFF"));
        CHECK_EQ(big_integer("10", 16, true).data, cpp_int("-0x10"));
        CHECK_EQ(big_integer("0", 16, false).data, cpp_int(0));
        CHECK_EQ(big_integer("123456789ABCDEF0", 16, false).data, cpp_int("0x123456789ABCDEF0"));
      }

      SUBCASE("Radix 36")
      {
        /* 1bz9 (base 36) = 1*36^3 + 11*36^2 + 35*36^1 + 9*36^0 = 46656 + 14256 + 1260 + 9 = 62181 */
        CHECK_EQ(big_integer("1bz9", 36, false).data, cpp_int(62181));
        /* za (base 36) = 35*36^1 + 10*36^0 = 1260 + 10 = 1270 */
        CHECK_EQ(big_integer("za", 36, true).data, cpp_int(-1270));
        CHECK_EQ(big_integer("helloworld", 36, false).data, 1767707668033969);
        CHECK_EQ(big_integer("0", 36, false).data, cpp_int(0));
      }
    }
  }
}
