#include <jank/util/string_builder.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::util
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
        native_transient_string const input(initial_capacity / 2, ' ');
        CHECK_EQ(input, sb.view());
      }

      SUBCASE("no resize: full")
      {
        string_builder sb;
        native_transient_string const input(initial_capacity - 1, 'a');
        sb(input);
        CHECK_EQ(input.size(), sb.pos);
        CHECK_EQ(initial_capacity, sb.capacity);
        CHECK_EQ(input, sb.view());
      }

      SUBCASE("resize: full + 1")
      {
        string_builder sb;
        native_transient_string const input(initial_capacity, 'a');
        sb(input);
        CHECK_EQ(input.size(), sb.pos);
        CHECK_EQ(initial_capacity * 2, sb.capacity);
        CHECK_EQ(input, sb.view());
      }

      SUBCASE("resize: full + a bunch")
      {
        string_builder sb;
        native_transient_string const input((initial_capacity * 3) + 5, '.');
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
      CHECK_EQ(8, sb.pos);
      CHECK_EQ(initial_capacity, sb.capacity);
      CHECK_EQ("3.140000", sb.view());
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
