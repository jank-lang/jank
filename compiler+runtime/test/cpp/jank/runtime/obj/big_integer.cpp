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
  native_big_integer nbi(long long const val)
  {
    return native_big_integer(val);
  }

  native_big_integer nbi(char const *s)
  {
    return native_big_integer(s);
  }

  TEST_SUITE("big_integer")
  {
    TEST_CASE("Constructors")
    {
      SUBCASE("Default constructor")
      {
        big_integer bi;
        CHECK_EQ(bi.data, 0);
      }

      SUBCASE("native_big_integer const& constructor")
      {
        native_big_integer val = nbi("12345678901234567890");
        big_integer bi(val);
        CHECK_EQ(bi.data, val);
      }

      SUBCASE("native_big_integer && constructor")
      {
        native_big_integer val = nbi("-98765432109876543210");
        big_integer bi(std::move(val));
        CHECK_EQ(bi.data, nbi("-98765432109876543210"));
      }

      SUBCASE("native_integer constructor")
      {
        big_integer bi(12345LL);
        CHECK_EQ(bi.data, 12345);
        big_integer bi_neg(-67890LL);
        CHECK_EQ(bi_neg.data, -67890);
      }

      SUBCASE("string constructor - valid")
      {
        big_integer bi("999999999999999999999999999999");
        CHECK_EQ(bi.data, nbi("999999999999999999999999999999"));
        big_integer bi_neg("-100000000000000000000");
        CHECK_EQ(bi_neg.data, nbi("-100000000000000000000"));
        big_integer bi_zero("0");
        CHECK_EQ(bi_zero.data, 0);
        big_integer bi_pad("007");
        CHECK_EQ(bi_pad.data, 7);
      }

      SUBCASE("string constructor - invalid")
      {
        CHECK_THROWS_AS(big_integer("abc"), std::runtime_error);
        CHECK_THROWS_AS(big_integer("123a"), std::runtime_error);
        CHECK_THROWS_AS(big_integer(""), std::runtime_error);
        CHECK_THROWS_AS(big_integer("--1"), std::runtime_error);
      }
    }

    TEST_CASE("equal method")
    {
      auto bi1 = make_box<big_integer>(nbi("123456789012345"));
      auto bi2 = make_box<big_integer>(nbi("123456789012345"));
      auto bi3 = make_box<big_integer>(nbi("-123456789012345"));
      auto bi_zero = make_box<big_integer>(0);
      auto i1 = make_box<integer>(12345LL);
      auto i2 = make_box<integer>(12345LL);
      auto i3 = make_box<integer>(-12345LL);
      auto r1 = make_box<real>(12345.0);
      auto r2 = make_box<real>(12345.00000000001);
      auto r3 = make_box<real>(12345.0000000000000001);
      auto r_large_exact = make_box<real>(123456789012345.0);
      auto r_large_approx = make_box<real>(123456789012345.00001);
      auto ratio1 = make_box<obj::ratio>(obj::ratio_data(1, 2));

      SUBCASE("big_integer vs big_integer")
      {
        CHECK(bi1->equal(*erase(bi2)));
        CHECK_FALSE(bi1->equal(*erase(bi3)));
        CHECK_FALSE(bi1->equal(*erase(bi_zero)));
      }

      SUBCASE("big_integer vs integer")
      {
        big_integer bi_i1(12345LL);
        big_integer bi_i3(-12345LL);
        CHECK(bi_i1.equal(*erase(i1)));
        CHECK(bi_i1.equal(*erase(i2)));
        CHECK_FALSE(bi_i1.equal(*erase(i3)));
        CHECK(bi_i3.equal(*erase(i3)));
        CHECK_FALSE(bi_i3.equal(*erase(i1)));
      }

      SUBCASE("big_integer vs real (epsilon comparison)")
      {
        big_integer bi_r1(12345LL);
        big_integer bi_r_large(nbi("123456789012345"));

        CHECK(bi_r1.equal(*erase(r1)));
        CHECK_FALSE(bi_r1.equal(*erase(r2)));
        CHECK(bi_r1.equal(*erase(r3)));


        CHECK(bi_r_large.equal(*erase(r_large_exact)));
        CHECK(bi_r_large.equal(*erase(r_large_approx)));

        /* Numbers that cause overflow during conversion in `equal`. */
        /* Create a BI larger than double can represent. */
        native_big_integer huge_val = nbi("1") << 1024;
        big_integer bi_huge_pos(huge_val);
        big_integer bi_huge_neg(-huge_val);
        auto r_inf_pos = make_box<real>(std::numeric_limits<native_real>::infinity());
        auto r_inf_neg = make_box<real>(-std::numeric_limits<native_real>::infinity());

        /* Conversion works, but comparison should use epsilon logic which fails for inf. */
        CHECK_FALSE(bi_huge_pos.equal(*erase(r_inf_pos)));
        CHECK_FALSE(bi_huge_neg.equal(*erase(r_inf_neg)));
        CHECK_FALSE(bi_huge_pos.equal(*erase(r1)));
      }

      SUBCASE("big_integer vs other type")
      {
        CHECK_FALSE(bi1->equal(*erase(ratio1)));
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
        CHECK_EQ(sb1.view(), "1234567890987654321");

        util::string_builder sb2;
        bi_neg.to_string(sb2);
        CHECK_EQ(sb2.view(), "-9876543210123456789");

        util::string_builder sb3;
        bi_zero.to_string(sb3);
        CHECK_EQ(sb3.view(), "0");
      }
    }

    TEST_CASE("Hashing")
    {
      big_integer bi1(nbi("12345678901234567890"));
      big_integer bi2(nbi("12345678901234567890"));
      big_integer bi3(nbi("-12345678901234567890"));
      big_integer bi4(0);
      big_integer bi5(-1);

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
      auto bi_10 = make_box<big_integer>(10);
      auto bi_20 = make_box<big_integer>(20);
      auto bi_10_neg = make_box<big_integer>(-10);
      auto bi_large = make_box<big_integer>(nbi("100000000000000000000"));

      auto i_15 = make_box<integer>(15);
      auto i_10 = make_box<integer>(10);
      auto i_10_neg = make_box<integer>(-10);

      auto r_10 = make_box<real>(10.0);
      auto r_10_5 = make_box<real>(10.5);
      auto r_neg_10 = make_box<real>(-10.0);
      auto r_neg_9_5 = make_box<real>(-9.5);

      auto ratio_half = make_box<obj::ratio>(obj::ratio_data(1, 2));
      auto bool_true = make_box<obj::boolean>(true);

      SUBCASE("big_integer vs big_integer")
      {
        CHECK_LT(bi_10->compare(*erase(bi_20)), 0);
        CHECK_GT(bi_20->compare(*erase(bi_10)), 0);
        CHECK_EQ(bi_10->compare(*erase(make_box<big_integer>(10))), 0);
        CHECK_GT(bi_10->compare(*erase(bi_10_neg)), 0);
        CHECK_LT(bi_10_neg->compare(*erase(bi_10)), 0);
        CHECK_GT(bi_large->compare(*erase(bi_20)), 0);
      }

      SUBCASE("big_integer vs integer")
      {
        CHECK_LT(bi_10->compare(*erase(i_15)), 0);
        CHECK_EQ(bi_10->compare(*erase(i_10)), 0);
        CHECK_GT(bi_10->compare(*erase(i_10_neg)), 0);
        CHECK_GT(bi_large->compare(*erase(make_box<integer>(LLONG_MAX))), 0);
      }

      SUBCASE("big_integer vs real")
      {
        CHECK_LT(bi_10->compare(*erase(r_10_5)), 0);
        CHECK_EQ(bi_10->compare(*erase(r_10)), 0);
        CHECK_GT(bi_10->compare(*erase(r_neg_10)), 0);
        CHECK_LT(bi_10_neg->compare(*erase(r_neg_9_5)), 0);
        CHECK_GT(bi_large->compare(*erase(make_box<real>(static_cast<native_real>(LLONG_MAX)))), 0);
      }

      SUBCASE("big_integer vs ratio")
      {
        CHECK_GT(bi_10->compare(*erase(ratio_half)), 0);
        CHECK_EQ(make_box<big_integer>(0)->compare(*erase(ratio_half)), -1);
      }

      SUBCASE("big_integer vs incompatible")
      {
        CHECK_THROWS_AS(bi_10->compare(*erase(bool_true)), std::runtime_error);
      }
    }

    TEST_CASE("Static gcd")
    {
      CHECK_EQ(big_integer::gcd(nbi(12), nbi(18)), 6);
      CHECK_EQ(big_integer::gcd(nbi(-12), nbi(18)), 6);
      CHECK_EQ(big_integer::gcd(nbi(12), nbi(-18)), 6);
      CHECK_EQ(big_integer::gcd(nbi(-12), nbi(-18)), 6);
      CHECK_EQ(big_integer::gcd(nbi(17), nbi(5)), 1);
      CHECK_EQ(big_integer::gcd(nbi(0LL), nbi(5)), 5);
      CHECK_EQ(big_integer::gcd(nbi(5), nbi(0LL)), 5);
      CHECK_EQ(big_integer::gcd(nbi(0LL), nbi(0LL)), 0LL);
      CHECK_EQ(big_integer::gcd(nbi(1), nbi(5)), 1);
      CHECK_EQ(big_integer::gcd(nbi(5), nbi(1)), 1);
      CHECK_EQ(big_integer::gcd(nbi("123456789012"), nbi("987654321098")), 2);
    }

    TEST_CASE("to_integer / to_native_integer")
    {
      SUBCASE("Within native_integer range")
      {
        CHECK_EQ(big_integer(123).to_integer(), 123LL);
        CHECK_EQ(big_integer(0).to_integer(), 0LL);
        CHECK_EQ(big_integer(-456).to_integer(), -456LL);
        CHECK_EQ(big_integer(LLONG_MAX).to_integer(), LLONG_MAX);
        CHECK_EQ(big_integer(LLONG_MIN).to_integer(), LLONG_MIN);
      }

      SUBCASE("Outside native_integer range (throws)")
      {
        native_big_integer max_plus_1 = nbi(LLONG_MAX);
        ++max_plus_1;
        native_big_integer min_minus_1 = nbi(LLONG_MIN);
        --min_minus_1;

        CHECK_THROWS_AS(big_integer(max_plus_1).to_integer(), std::runtime_error);
        CHECK_THROWS_AS(big_integer(min_minus_1).to_integer(), std::runtime_error);

        native_big_integer very_large = nbi(1) << 100; /* 2^100 */
        CHECK_THROWS_AS(big_integer(very_large).to_integer(), std::runtime_error);
        CHECK_THROWS_AS(big_integer(-very_large).to_integer(), std::runtime_error);
      }
    }

    TEST_CASE("to_real / to_native_real")
    {
      SUBCASE("Within native_real range")
      {
        CHECK_EQ(big_integer(123).to_real(), doctest::Approx(123.0));
        CHECK_EQ(big_integer(0).to_real(), doctest::Approx(0.0));
        CHECK_EQ(big_integer(-456).to_real(), doctest::Approx(-456.0));
        /* A large number exactly representable by double. */
        native_big_integer const exact_double_val = nbi(1) << 50;
        CHECK_EQ(big_integer(exact_double_val).to_real(), doctest::Approx(std::pow(2.0, 50)));
      }

      SUBCASE("Outside native_real range (infinity)")
      {
        native_big_integer const huge_pos = nbi(1) << 1024; /* 2^1024 > DBL_MAX */
        native_big_integer const huge_neg = -huge_pos;
        CHECK_EQ(big_integer(huge_pos).to_real(), std::numeric_limits<native_real>::infinity());
        CHECK_EQ(big_integer(huge_neg).to_real(), -std::numeric_limits<native_real>::infinity());
      }
    }

    TEST_CASE("Operators (big_integer vs native_real)")
    {
      using namespace jank::runtime;

      native_big_integer const bi_10 = nbi(10);
      native_big_integer const bi_neg_5 = nbi(-5);
      native_big_integer const bi_large = nbi("10000000000000000");
      constexpr native_real r_3_5 = 3.5;
      constexpr native_real r_10 = 10.0;
      constexpr native_real r_large = 1.0e16;

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
        constexpr native_real r_neg_2 = -2.0;
        CHECK_EQ(operator/(bi_10, r_neg_2), doctest::Approx(-5.0));
        CHECK_EQ(operator/(r_neg_2, bi_10), doctest::Approx(-0.2));
        CHECK_EQ(operator/(bi_neg_5, r_neg_2), doctest::Approx(2.5));
        CHECK_EQ(operator/(r_neg_2, bi_neg_5), doctest::Approx(0.4));
      }
      SUBCASE("Equality (epsilon)")
      {
        constexpr native_real epsilon = std::numeric_limits<native_real>::epsilon();
        native_real r_10_eps = 10.0 + epsilon / 2.0;
        native_real r_10_10eps = 10.0 + epsilon * 10.0;

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

        cpp_int expected_large_bin = (cpp_int(1) << 64) - 1; /* 2^64 - 1 */
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

      SUBCASE("Radix 10 (Decimal)")
      {
        CHECK_EQ(big_integer("12345678901234567890", 10, false).data,
                 cpp_int("12345678901234567890"));
        CHECK_EQ(big_integer("9876543210", 10, true).data, cpp_int("-9876543210"));
        CHECK_EQ(big_integer("0", 10, false).data, cpp_int(0));
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

      SUBCASE("Large Numbers (Manual Construction)")
      {
        cpp_int const expected_bin_100_ones = (cpp_int(1) << 100) - 1;
        std::string const large_bin(100, '1');
        CHECK_EQ(big_integer(large_bin, 2, false).data, expected_bin_100_ones);

        cpp_int const expected_hex_1_30_zeros = cpp_int(1) << (30 * 4); // 1 * 16^30
        std::string large_hex = "1";
        large_hex.append(30, '0');
        CHECK_EQ(big_integer(large_hex, 16, false).data, expected_hex_1_30_zeros);

        /* Calculate a large base 36 number 'ghijklmnopqrstuvwxyz' */
        cpp_int expected_large_36 = 0;
        std::string large_36_str = "ghijklmnopqrstuvwxyz";
        cpp_int power_36 = 1;
        for(auto it = large_36_str.rbegin(); it != large_36_str.rend(); ++it)
        {
          int digit_val = (*it) - 'a' + 10;
          expected_large_36 += power_36 * digit_val;
          if(it + 1 != large_36_str.rend())
          {
            power_36 *= 36;
          }
        }
        CHECK_EQ(big_integer(large_36_str, 36, true).data, -expected_large_36);
      }
    }
  }
}
