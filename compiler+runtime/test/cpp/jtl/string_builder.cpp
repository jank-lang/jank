#include <jtl/string_builder.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jtl
{
  static constexpr usize initial_capacity{ string_builder::initial_capacity };

  TEST_SUITE("string_builder")
  {
    TEST_CASE("empty")
    {
      string_builder const sb;
      CHECK_EQ("", sb.view());
    }

    TEST_CASE("resize")
    {
      SUBCASE("no resize: more space")
      {
        string_builder sb;
        for(usize i{}; i < initial_capacity / 2; ++i)
        {
          sb(' ');
        }
        CHECK_EQ(initial_capacity / 2, sb.pos);
        CHECK_EQ(initial_capacity, sb.capacity);
        jtl::immutable_string const input(initial_capacity / 2, ' ');
        CHECK_EQ(input, sb.view());
      }

      SUBCASE("no resize: full")
      {
        string_builder sb;
        jtl::immutable_string const input(initial_capacity - 1, 'a');
        sb(input);
        CHECK_EQ(input.size(), sb.pos);
        CHECK_EQ(initial_capacity, sb.capacity);
        CHECK_EQ(input, sb.view());
      }

      SUBCASE("resize: full + 1")
      {
        string_builder sb;
        jtl::immutable_string const input(initial_capacity, 'a');
        sb(input);
        CHECK_EQ(input.size(), sb.pos);
        CHECK_EQ(initial_capacity * 2, sb.capacity);
        CHECK_EQ(input, sb.view());
      }

      SUBCASE("resize: full + a bunch")
      {
        string_builder sb;
        jtl::immutable_string const input((initial_capacity * 3) + 5, '.');
        sb(input);
        CHECK_EQ(input.size(), sb.pos);
        CHECK_EQ(initial_capacity * 4, sb.capacity);
        CHECK_EQ(input, sb.view());
      }
    }

    TEST_CASE("pointer")
    {
      string_builder sb;
      auto const p{ reinterpret_cast<void const *>(0xcafebabe) };
      sb(p);
      CHECK_EQ(10, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("0xcafebabe", sb.view());
    }

    TEST_CASE("int")
    {
      string_builder sb;
      sb(-50000);
      CHECK_EQ(6, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("-50000", sb.view());
    }

    TEST_CASE("float / double")
    {
      string_builder sb;
      sb(3.14);
      CHECK_EQ(4, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("3.14", sb.view());

      sb.pos = 0;
      sb(2989000001.0);
      CHECK_EQ(12, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("2989000001.0", sb.view());

      sb.pos = 0;
      sb(9007199254740992.0);
      CHECK_EQ(18, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("9007199254740992.0", sb.view());
    }

    TEST_CASE("float / double scientific")
    {
      string_builder sb;
      sb(1e-9);
      CHECK_EQ(5, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("1e-09", sb.view());
    }

    TEST_CASE("float / double scientific from non-scientific")
    {
      string_builder sb;
      sb(0.000000000000000000000000000001);
      CHECK_EQ(5, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("1e-30", sb.view());
    }

    TEST_CASE("infinity")
    {
      string_builder sb;
      sb(INFINITY);
      CHECK_EQ(8, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("INFINITY", sb.view());
    }

    TEST_CASE("nan")
    {
      string_builder sb;
      sb(NAN);
      CHECK_EQ(3, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("NAN", sb.view());
    }

    TEST_CASE("char32_t")
    {
      string_builder sb;
      sb(static_cast<char32_t>(0x1F601));
      CHECK_EQ(4, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("😁", sb.view());
    }
  }
}
