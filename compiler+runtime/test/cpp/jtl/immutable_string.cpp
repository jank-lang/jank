#include <jtl/immutable_string.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank
{
  TEST_SUITE("jtl::immutable_string"){
    TEST_CASE("Constructor"){ SUBCASE("Default"){ jtl::immutable_string const s;
  CHECK(s.empty());
}

SUBCASE("Copy")
{
  SUBCASE("SSO")
  {
    jtl::immutable_string const s{ "foo bar" };
    /* NOLINTNEXTLINE(performance-unnecessary-copy-initialization) */
    jtl::immutable_string const c{ s };
    CHECK_EQ(c.size(), 7);
    CHECK_NE(c.data(), s.data());
  }

  SUBCASE("Long")
  {
    jtl::immutable_string const s{ "foo bar spam meow foo bar spam meow" };
    /* NOLINTNEXTLINE(performance-unnecessary-copy-initialization) */
    jtl::immutable_string const c{ s };
    CHECK_EQ(c.size(), 35);
    CHECK_EQ(c.data(), s.data());
  }
}

SUBCASE("C string")
{
  SUBCASE("SSO")
  {
    jtl::immutable_string const s{ "foo bar" };
    CHECK_EQ(s.size(), 7);
  }

  SUBCASE("Long")
  {
    jtl::immutable_string const s{ "foo bar spam foo bar spam foo bar spam" };
    CHECK_EQ(s.size(), 38);
  }
}
}

TEST_CASE("Find")
{
  SUBCASE("Empty corpus, empty pattern")
  {
    jtl::immutable_string const s;
    CHECK_EQ(s.find(""), 0);
  }

  SUBCASE("Empty corpus")
  {
    jtl::immutable_string const s;
    CHECK_EQ(s.find("something"), jtl::immutable_string::npos);
  }

  SUBCASE("Empty corpus, high pos")
  {
    jtl::immutable_string const s;
    CHECK_EQ(s.find("something", 10), jtl::immutable_string::npos);
  }

  SUBCASE("Non-empty corpus, empty pattern")
  {
    jtl::immutable_string const s{ "foo bar" };
    CHECK_EQ(s.find(""), 0);
  }

  SUBCASE("Non-empty corpus, missing pattern")
  {
    jtl::immutable_string const s{ "I'm not Abel. I'm just Cain." };
    CHECK_EQ(s.find("p"), jtl::immutable_string::npos);
  }

  SUBCASE("Non-empty corpus, missing pattern longer than corpus")
  {
    jtl::immutable_string const s{ "I'm not Abel. I'm just Cain." };
    CHECK_EQ(s.find("Cupiditate eveniet at alias amet. Placeat facere qui sunt vel voluptas "
                    "tenetur. Sunt molestias exercitationem repellat aut non qui. Exercitationem "
                    "iste similique similique ut."),
             jtl::immutable_string::npos);
  }
}

TEST_CASE("Substring")
{
  SUBCASE("Empty corpus, empty substring")
  {
    jtl::immutable_string const s;
    auto const sub(s.substr());
    CHECK(sub.empty());
  }

  SUBCASE("Empty corpus, pos > 0")
  {
    jtl::immutable_string const s;
    CHECK_THROWS(s.substr(1));
  }

  SUBCASE("Empty corpus, pos = count, high count")
  {
    jtl::immutable_string const s;
    auto const sub(s.substr(0, 100));
    CHECK(sub.empty());
  }

  SUBCASE("Non-empty corpus, pos = count, high count")
  {
    jtl::immutable_string const s{ "foo bar" };
    auto const sub(s.substr(7, 100));
    CHECK(sub.empty());
  }

  SUBCASE("Non-empty corpus, pos = count, high count")
  {
    jtl::immutable_string const s{ "foo bar" };
    auto const sub(s.substr(7, 100));
    CHECK(sub.empty());
  }

  SUBCASE("SSO")
  {
    SUBCASE("Non-empty corpus, prefix")
    {
      jtl::immutable_string const s{ "foo bar" };
      auto const sub(s.substr(0, 3));
      CHECK_EQ(sub.size(), 3);
      CHECK_EQ(sub, "foo");
    }

    SUBCASE("Non-empty corpus, suffix")
    {
      jtl::immutable_string const s{ "foo bar" };
      auto const sub(s.substr(4, 3));
      CHECK_EQ(sub.size(), 3);
      CHECK_EQ(sub, "bar");
      auto const sub2(s.substr(4));
      CHECK_EQ(sub2.size(), 3);
      CHECK_EQ(sub, sub2);
    }

    SUBCASE("Non-empty corpus, middle")
    {
      jtl::immutable_string const s{ "foo bar" };
      auto const sub(s.substr(2, 3));
      CHECK_EQ(sub.size(), 3);
      CHECK_EQ(sub, "o b");
    }
  }

  SUBCASE("Long")
  {
    SUBCASE("Non-empty corpus, prefix")
    {
      jtl::immutable_string const s{ "foo bar spam meow foo bar spam meow" };
      auto const sub(s.substr(0, 3));
      CHECK_EQ(sub.size(), 3);
      CHECK_EQ(sub, "foo");
    }

    SUBCASE("Non-empty corpus, suffix")
    {
      jtl::immutable_string const s{ "foo bar spam meow foo bar spam meow" };
      auto const sub(s.substr(4, 3));
      CHECK_EQ(sub.size(), 3);
      CHECK_EQ(sub, "bar");
      auto const sub2(s.substr(4));
      CHECK_EQ(sub2, "bar spam meow foo bar spam meow");
      CHECK_EQ(sub2.substr(0, 3), sub);
    }

    SUBCASE("Non-empty corpus, middle")
    {
      jtl::immutable_string const s{ "foo bar spam meow foo bar spam meow" };
      auto const sub(s.substr(2, 3));
      CHECK_EQ(sub.size(), 3);
      CHECK_EQ(sub, "o b");
    }
  }
}
}
;
}
