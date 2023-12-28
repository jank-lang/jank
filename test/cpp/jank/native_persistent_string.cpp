#include <jank/native_persistent_string.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank
{
  TEST_SUITE("native_persistent_string")
  {
    TEST_CASE("Constructor")
    {
      SUBCASE("Default")
      {
        native_persistent_string s;
        CHECK(s.empty());
      }

      SUBCASE("Copy")
      {
        SUBCASE("SSO")
        {
          native_persistent_string s{ "foo bar" };
          native_persistent_string c{ s };
          CHECK_EQ(c.size(), 7);
          CHECK_NE(c.data(), s.data());
        }

        SUBCASE("Long")
        {
          native_persistent_string s{ "foo bar spam meow foo bar spam meow" };
          native_persistent_string c{ s };
          CHECK_EQ(c.size(), 35);
          CHECK_EQ(c.data(), s.data());
        }
      }

      SUBCASE("C string")
      {
        SUBCASE("SSO")
        {
          native_persistent_string s{ "foo bar" };
          CHECK_EQ(s.size(), 7);
        }

        SUBCASE("Long")
        {
          native_persistent_string s{ "foo bar spam foo bar spam foo bar spam" };
          CHECK_EQ(s.size(), 38);
        }
      }
    }

    TEST_CASE("Find")
    {
      SUBCASE("Empty corpus, empty pattern")
      {
        native_persistent_string s;
        CHECK_EQ(s.find(""), 0);
      }

      SUBCASE("Empty corpus")
      {
        native_persistent_string s;
        CHECK_EQ(s.find("something"), native_persistent_string::npos);
      }

      SUBCASE("Empty corpus, high pos")
      {
        native_persistent_string s;
        CHECK_EQ(s.find("something", 10), native_persistent_string::npos);
      }

      SUBCASE("Non-empty corpus, empty pattern")
      {
        native_persistent_string s{ "foo bar" };
        CHECK_EQ(s.find(""), 0);
      }

      SUBCASE("Non-empty corpus, missing pattern")
      {
        native_persistent_string s{ "I'm not Abel. I'm just Cain." };
        CHECK_EQ(s.find("p"), native_persistent_string::npos);
      }

      SUBCASE("Non-empty corpus, missing pattern longer than corpus")
      {
        native_persistent_string s{ "I'm not Abel. I'm just Cain." };
        CHECK_EQ(s.find("Cupiditate eveniet at alias amet. Placeat facere qui sunt vel voluptas tenetur. Sunt molestias exercitationem repellat aut non qui. Exercitationem iste similique similique ut."), native_persistent_string::npos);
      }
    }

    TEST_CASE("Substring")
    {
      SUBCASE("Empty corpus, empty substring")
      {
        native_persistent_string s;
        auto const sub(s.substr());
        CHECK(sub.empty());
      }

      SUBCASE("Empty corpus, pos > 0")
      {
        native_persistent_string s;
        CHECK_THROWS(s.substr(1));
      }

      SUBCASE("Empty corpus, pos = count, high count")
      {
        native_persistent_string s;
        auto const sub(s.substr(0, 100));
        CHECK(sub.empty());
      }

      SUBCASE("Non-empty corpus, pos = count, high count")
      {
        native_persistent_string s{ "foo bar" };
        auto const sub(s.substr(7, 100));
        CHECK(sub.empty());
      }

      SUBCASE("Non-empty corpus, pos = count, high count")
      {
        native_persistent_string s{ "foo bar" };
        auto const sub(s.substr(7, 100));
        CHECK(sub.empty());
      }

      SUBCASE("SSO")
      {
        SUBCASE("Non-empty corpus, prefix")
        {
          native_persistent_string s{ "foo bar" };
          auto const sub(s.substr(0, 3));
          CHECK_EQ(sub.size(), 3);
          CHECK_EQ(sub, "foo");
        }

        SUBCASE("Non-empty corpus, suffix")
        {
          native_persistent_string s{ "foo bar" };
          auto const sub(s.substr(4, 3));
          CHECK_EQ(sub.size(), 3);
          CHECK_EQ(sub, "bar");
          auto const sub2(s.substr(4));
          CHECK_EQ(sub2.size(), 3);
          CHECK_EQ(sub, sub2);
        }

        SUBCASE("Non-empty corpus, middle")
        {
          native_persistent_string s{ "foo bar" };
          auto const sub(s.substr(2, 3));
          CHECK_EQ(sub.size(), 3);
          CHECK_EQ(sub, "o b");
        }
      }

      SUBCASE("Long")
      {
        SUBCASE("Non-empty corpus, prefix")
        {
          native_persistent_string s{ "foo bar spam meow foo bar spam meow" };
          auto const sub(s.substr(0, 3));
          CHECK_EQ(sub.size(), 3);
          CHECK_EQ(sub, "foo");
        }

        SUBCASE("Non-empty corpus, suffix")
        {
          native_persistent_string s{ "foo bar spam meow foo bar spam meow" };
          auto const sub(s.substr(4, 3));
          CHECK_EQ(sub.size(), 3);
          CHECK_EQ(sub, "bar");
          auto const sub2(s.substr(4));
          CHECK_EQ(sub2, "bar spam meow foo bar spam meow");
          CHECK_EQ(sub2.substr(0, 3), sub);
        }

        SUBCASE("Non-empty corpus, middle")
        {
          native_persistent_string s{ "foo bar spam meow foo bar spam meow" };
          auto const sub(s.substr(2, 3));
          CHECK_EQ(sub.size(), 3);
          CHECK_EQ(sub, "o b");
        }
      }
    }
  };
}
