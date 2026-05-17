/* The prelude is the C++ interop surface users get by default; it must expose the
 * built-in immer conversion traits without requiring a private converter include. */
#include <jank/prelude.hpp>
#include <jank/runtime/core/equal.hpp>

#include <immer/atom.hpp>

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::test::runtime::convert::immer
{
  struct row
  {
    int id{};
    int score{};
  };

  struct row_key_fn
  {
    int operator()(row const &r) const
    {
      return r.id;
    }

    row operator()(row r, int const key) const
    {
      r.id = key;
      return r;
    }
  };

  struct getter_only_row_key_fn
  {
    int operator()(row const &r) const
    {
      return r.id;
    }
  };
}

namespace jank::runtime
{
  template <>
  struct convert<jank::test::runtime::convert::immer::row>
  {
    static object_ref into_object(jank::test::runtime::convert::immer::row const &r)
    {
      return make_box<obj::persistent_vector>(std::in_place,
                                              convert<int>::into_object(r.id),
                                              convert<int>::into_object(r.score))
        .erase();
    }

    static jank::test::runtime::convert::immer::row from_object(object_ref const o)
    {
      auto const v{ try_object<obj::persistent_vector>(o) };
      return { convert<int>::from_object(v->data[0]), convert<int>::from_object(v->data[1]) };
    }
  };
}

namespace jank::test::runtime::convert::immer
{
  namespace rt = jank::runtime;

  struct non_convertible
  {
  };

  struct non_convertible_hash
  {
    std::size_t operator()(non_convertible const &) const
    {
      return 0;
    }
  };

  struct non_convertible_equal
  {
    bool operator()(non_convertible const &, non_convertible const &) const
    {
      return true;
    }
  };

  struct non_convertible_key_row
  {
    non_convertible id{};
    int score{};
  };

  struct non_convertible_key_fn
  {
    non_convertible operator()(non_convertible_key_row const &r) const
    {
      return r.id;
    }

    non_convertible_key_row operator()(non_convertible_key_row r, non_convertible const key) const
    {
      r.id = key;
      return r;
    }
  };

  struct non_convertible_row_key_fn
  {
    int operator()(non_convertible const &) const
    {
      return 0;
    }

    non_convertible operator()(non_convertible r, int const) const
    {
      return r;
    }
  };

  using vector_type = ::immer::vector<int, jank::memory_policy>;
  using vector_transient_type = ::immer::vector_transient<int, jank::memory_policy>;
  using flex_vector_type = ::immer::flex_vector<int, jank::memory_policy>;
  using flex_vector_transient_type = ::immer::flex_vector_transient<int, jank::memory_policy>;
  using dvektor_type = ::immer::dvektor<int, 5, jank::memory_policy>;
  using array_type = ::immer::array<int, jank::memory_policy>;
  using array_transient_type = ::immer::array_transient<int, jank::memory_policy>;
  using box_type = ::immer::box<int, jank::memory_policy>;
  using map_type = ::immer::map<int, int, std::hash<int>, std::equal_to<int>, jank::memory_policy>;
  using object_map_type = ::immer::map<rt::object_ref,
                                       rt::object_ref,
                                       std::hash<rt::object_ref>,
                                       std::equal_to<rt::object_ref>,
                                       jank::memory_policy>;
  using map_transient_type
    = ::immer::map_transient<int, int, std::hash<int>, std::equal_to<int>, jank::memory_policy>;
  using set_type = ::immer::set<int, std::hash<int>, std::equal_to<int>, jank::memory_policy>;
  using set_transient_type
    = ::immer::set_transient<int, std::hash<int>, std::equal_to<int>, jank::memory_policy>;
  using table_type
    = ::immer::table<row, row_key_fn, std::hash<int>, std::equal_to<int>, jank::memory_policy>;
  using table_transient_type = ::immer::table_transient<row,
                                                        row_key_fn,
                                                        std::hash<int>,
                                                        std::equal_to<int>,
                                                        jank::memory_policy,
                                                        ::immer::default_bits>;
  using atom_type = ::immer::atom<vector_type, jank::memory_policy>;
  using default_policy_vector_type = ::immer::vector<int>;
  using default_policy_vector_transient_type = ::immer::vector_transient<int>;
  using default_policy_flex_vector_type = ::immer::flex_vector<int>;
  using default_policy_flex_vector_transient_type = ::immer::flex_vector_transient<int>;
  using default_policy_dvektor_type = ::immer::dvektor<int>;
  using default_policy_array_type = ::immer::array<int>;
  using default_policy_array_transient_type = ::immer::array_transient<int>;
  using default_policy_box_type = ::immer::box<int>;
  using default_policy_map_type = ::immer::map<int, int>;
  using default_policy_map_transient_type = ::immer::map_transient<int, int>;
  using default_policy_set_type = ::immer::set<int>;
  using default_policy_set_transient_type = ::immer::set_transient<int>;
  using default_policy_table_type = ::immer::table<row, row_key_fn>;
  using default_policy_table_transient_type
    = ::immer::table_transient<row,
                               row_key_fn,
                               std::hash<int>,
                               std::equal_to<int>,
                               ::immer::default_memory_policy,
                               ::immer::default_bits>;
  using getter_only_table_type = ::immer::
    table<row, getter_only_row_key_fn, std::hash<int>, std::equal_to<int>, jank::memory_policy>;
  using getter_only_table_transient_type = ::immer::table_transient<row,
                                                                    getter_only_row_key_fn,
                                                                    std::hash<int>,
                                                                    std::equal_to<int>,
                                                                    jank::memory_policy,
                                                                    ::immer::default_bits>;

  static_assert(rt::convertible<vector_type>);
  static_assert(rt::convertible<vector_transient_type>);
  static_assert(rt::convertible<flex_vector_type>);
  static_assert(rt::convertible<flex_vector_transient_type>);
  static_assert(rt::convertible<dvektor_type>);
  static_assert(rt::convertible<array_type>);
  static_assert(rt::convertible<array_transient_type>);
  static_assert(rt::convertible<box_type>);
  static_assert(rt::convertible<map_type>);
  static_assert(rt::convertible<object_map_type>);
  static_assert(rt::convertible<map_transient_type>);
  static_assert(rt::convertible<set_type>);
  static_assert(rt::convertible<set_transient_type>);
  static_assert(rt::convertible<table_type>);
  static_assert(rt::convertible<table_transient_type>);
  static_assert(!rt::convertible<atom_type>);
  static_assert(!rt::convertible<default_policy_vector_type>);
  static_assert(!rt::convertible<default_policy_vector_transient_type>);
  static_assert(!rt::convertible<default_policy_flex_vector_type>);
  static_assert(!rt::convertible<default_policy_flex_vector_transient_type>);
  static_assert(!rt::convertible<default_policy_dvektor_type>);
  static_assert(!rt::convertible<default_policy_array_type>);
  static_assert(!rt::convertible<default_policy_array_transient_type>);
  static_assert(!rt::convertible<default_policy_box_type>);
  static_assert(!rt::convertible<default_policy_map_type>);
  static_assert(!rt::convertible<default_policy_map_transient_type>);
  static_assert(!rt::convertible<default_policy_set_type>);
  static_assert(!rt::convertible<default_policy_set_transient_type>);
  static_assert(!rt::convertible<default_policy_table_type>);
  static_assert(!rt::convertible<default_policy_table_transient_type>);
  static_assert(!rt::convertible<getter_only_table_type>);
  static_assert(!rt::convertible<getter_only_table_transient_type>);
  static_assert(!rt::convertible<::immer::vector<non_convertible, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::vector_transient<non_convertible, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::flex_vector<non_convertible, jank::memory_policy>>);
  static_assert(
    !rt::convertible<::immer::flex_vector_transient<non_convertible, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::dvektor<non_convertible, 5, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::array<non_convertible, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::array_transient<non_convertible, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::box<non_convertible, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::map<non_convertible,
                                              int,
                                              non_convertible_hash,
                                              non_convertible_equal,
                                              jank::memory_policy>>);
  static_assert(
    !rt::convertible<
      ::immer::map<int, non_convertible, std::hash<int>, std::equal_to<int>, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::map_transient<non_convertible,
                                                        int,
                                                        non_convertible_hash,
                                                        non_convertible_equal,
                                                        jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::map_transient<int,
                                                        non_convertible,
                                                        std::hash<int>,
                                                        std::equal_to<int>,
                                                        jank::memory_policy>>);
  static_assert(
    !rt::convertible<
      ::immer::
        set<non_convertible, non_convertible_hash, non_convertible_equal, jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::set_transient<non_convertible,
                                                        non_convertible_hash,
                                                        non_convertible_equal,
                                                        jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::table<non_convertible,
                                                non_convertible_row_key_fn,
                                                std::hash<int>,
                                                std::equal_to<int>,
                                                jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::table_transient<non_convertible,
                                                          non_convertible_row_key_fn,
                                                          std::hash<int>,
                                                          std::equal_to<int>,
                                                          jank::memory_policy,
                                                          ::immer::default_bits>>);
  static_assert(!rt::convertible<::immer::table<non_convertible_key_row,
                                                non_convertible_key_fn,
                                                non_convertible_hash,
                                                non_convertible_equal,
                                                jank::memory_policy>>);
  static_assert(!rt::convertible<::immer::table_transient<non_convertible_key_row,
                                                          non_convertible_key_fn,
                                                          non_convertible_hash,
                                                          non_convertible_equal,
                                                          jank::memory_policy,
                                                          ::immer::default_bits>>);

  static_assert(std::same_as<decltype(rt::convert<vector_type>::into_object(
                               std::declval<vector_type const &>())),
                             rt::obj::persistent_vector_ref>);
  static_assert(
    std::same_as<decltype(rt::convert<map_type>::into_object(std::declval<map_type const &>())),
                 rt::obj::persistent_hash_map_ref>);
  static_assert(
    std::same_as<decltype(rt::convert<set_type>::into_object(std::declval<set_type const &>())),
                 rt::obj::persistent_hash_set_ref>);
  static_assert(
    std::same_as<decltype(rt::convert<table_type>::into_object(std::declval<table_type const &>())),
                 rt::obj::persistent_hash_map_ref>);

  TEST_CASE("immer converter header interop contract")
  {
    CHECK(rt::convertible<vector_type>);
    CHECK(rt::convertible<dvektor_type>);
    CHECK(rt::convertible<map_type>);
    CHECK(rt::convertible<set_type>);
    CHECK(rt::convertible<table_type>);
    CHECK_FALSE(rt::convertible<atom_type>);
    CHECK_FALSE(rt::convertible<default_policy_vector_type>);
    CHECK_FALSE(rt::convertible<default_policy_dvektor_type>);
    CHECK_FALSE(rt::convertible<default_policy_map_type>);
    CHECK_FALSE(rt::convertible<default_policy_set_type>);
    CHECK_FALSE(rt::convertible<default_policy_table_type>);
    CHECK_FALSE(rt::convertible<getter_only_table_type>);
    CHECK_FALSE(rt::convertible<getter_only_table_transient_type>);
  }

  TEST_CASE("immer converter runtime value contract")
  {
    auto const vector_object{ rt::make_box<rt::obj::persistent_vector>(std::in_place,
                                                                       rt::make_box(1),
                                                                       rt::make_box(2),
                                                                       rt::make_box(3)) };
    auto const vector_value{ rt::convert<vector_type>::from_object(vector_object.erase()) };
    CHECK(vector_value.size() == 3);
    CHECK(vector_value[0] == 1);
    CHECK(vector_value[2] == 3);

    auto const vector_transient_value{ rt::convert<vector_transient_type>::from_object(
      vector_object.erase()) };
    CHECK(vector_transient_value.size() == 3);
    CHECK(rt::equal(rt::convert<vector_type>::into_object(vector_value).erase(),
                    vector_object.erase()));

    auto const flex_vector_value{ rt::convert<flex_vector_type>::from_object(
      vector_object.erase()) };
    CHECK(flex_vector_value.size() == 3);
    CHECK(rt::equal(rt::convert<flex_vector_type>::into_object(flex_vector_value).erase(),
                    vector_object.erase()));

    auto const flex_vector_transient_value{ rt::convert<flex_vector_transient_type>::from_object(
      vector_object.erase()) };
    CHECK(flex_vector_transient_value.size() == 3);

    auto const dvektor_value{ rt::convert<dvektor_type>::from_object(vector_object.erase()) };
    CHECK(dvektor_value.size() == 3);
    CHECK(dvektor_value[1] == 2);
    CHECK(rt::equal(rt::convert<dvektor_type>::into_object(dvektor_value).erase(),
                    vector_object.erase()));
    auto const empty_vector_object{ rt::make_box<rt::obj::persistent_vector>(std::in_place) };
    auto const empty_dvektor_value{ rt::convert<dvektor_type>::from_object(
      empty_vector_object.erase()) };
    CHECK(empty_dvektor_value.empty());
    auto empty_dvektor_sum{ 0 };
    for(auto const i : empty_dvektor_value)
    {
      empty_dvektor_sum += i;
    }
    CHECK(empty_dvektor_sum == 0);
    CHECK(rt::equal(rt::convert<dvektor_type>::into_object(empty_dvektor_value).erase(),
                    empty_vector_object.erase()));

    auto const array_value{ rt::convert<array_type>::from_object(vector_object.erase()) };
    CHECK(array_value.size() == 3);
    CHECK(
      rt::equal(rt::convert<array_type>::into_object(array_value).erase(), vector_object.erase()));

    auto const array_transient_value{ rt::convert<array_transient_type>::from_object(
      vector_object.erase()) };
    CHECK(array_transient_value.size() == 3);

    auto const boxed_value{ rt::convert<box_type>::from_object(rt::make_box(8)) };
    CHECK(boxed_value.get() == 8);
    CHECK(rt::convert<int>::from_object(rt::convert<box_type>::into_object(boxed_value)) == 8);

    auto const map_object{ rt::obj::persistent_hash_map::create_unique(
      std::make_pair(rt::make_box(1), rt::make_box(10)),
      std::make_pair(rt::make_box(2), rt::make_box(20))) };
    auto const map_value{ rt::convert<map_type>::from_object(map_object.erase()) };
    CHECK(map_value.size() == 2);
    CHECK(map_value.at(2) == 20);
    CHECK(rt::convert<int>::from_object(
            rt::convert<map_type>::into_object(map_value)->get(rt::make_box(1)))
          == 10);

    auto const object_key{ rt::make_box<rt::obj::persistent_vector>(std::in_place,
                                                                    rt::make_box(9)) };
    auto const object_value{ rt::make_box<rt::obj::persistent_vector>(std::in_place,
                                                                      rt::make_box(90)) };
    auto const object_map_object{ rt::obj::persistent_hash_map::create_unique(
      std::make_pair(object_key.erase(), object_value.erase())) };
    auto const object_map_value{ rt::convert<object_map_type>::from_object(
      object_map_object.erase()) };
    CHECK(object_map_value.size() == 1);
    CHECK(rt::equal(object_map_value.at(object_key.erase()), object_value.erase()));
    CHECK(rt::equal(
      rt::convert<object_map_type>::into_object(object_map_value)->get(object_key.erase()),
      object_value.erase()));

    auto const entry_vector{ rt::make_box<rt::obj::persistent_vector>(
      std::in_place,
      rt::make_box<rt::obj::persistent_vector>(std::in_place, rt::make_box(3), rt::make_box(30))) };
    auto const transient_map_value{ rt::convert<map_transient_type>::from_object(
      entry_vector.erase()) };
    CHECK(transient_map_value.size() == 1);
    CHECK(transient_map_value.at(3) == 30);

    auto const scalar_entry_vector{ rt::make_box<rt::obj::persistent_vector>(std::in_place,
                                                                             rt::make_box(4)) };
    auto const malformed_entry_message{ "map entry must contain exactly two values" };
    auto const check_malformed_entry{ [&](auto const &fn) {
      try
      {
        fn();
        FAIL("expected malformed map entry");
      }
      catch(std::runtime_error const &e)
      {
        CHECK(std::string_view{ e.what() } == malformed_entry_message);
      }
    } };
    check_malformed_entry(
      [&] { static_cast<void>(rt::convert<map_type>::from_object(scalar_entry_vector.erase())); });
    check_malformed_entry([&] {
      static_cast<void>(rt::convert<map_transient_type>::from_object(scalar_entry_vector.erase()));
    });

    auto const set_object{
      rt::make_box<rt::obj::persistent_hash_set>(std::in_place, rt::make_box(2), rt::make_box(4))
    };
    auto const set_value{ rt::convert<set_type>::from_object(set_object.erase()) };
    CHECK(set_value.count(2) == 1);
    CHECK(set_value.count(5) == 0);

    auto const transient_set_value{ rt::convert<set_transient_type>::from_object(
      vector_object.erase()) };
    CHECK(transient_set_value.count(3) == 1);
    CHECK(transient_set_value.count(5) == 0);

    auto const table_object{ rt::obj::persistent_hash_map::create_unique(std::make_pair(
      rt::make_box(9),
      rt::make_box<rt::obj::persistent_vector>(std::in_place, rt::make_box(0), rt::make_box(90))
        .erase())) };
    auto const table_value{ rt::convert<table_type>::from_object(table_object.erase()) };
    CHECK(table_value.size() == 1);
    CHECK(table_value.at(9).id == 9);
    CHECK(table_value.at(9).score == 90);

    auto const transient_table_value{ rt::convert<table_transient_type>::from_object(
      table_object.erase()) };
    CHECK(transient_table_value.size() == 1);
    CHECK(transient_table_value.at(9).id == 9);
    CHECK(transient_table_value.at(9).score == 90);
    auto const table_round_trip{ rt::convert<table_type>::into_object(table_value) };
    auto const row_object{ rt::try_object<rt::obj::persistent_vector>(
      table_round_trip->get(rt::make_box(9))) };
    CHECK(rt::convert<int>::from_object(row_object->data[0]) == 9);
    CHECK(rt::convert<int>::from_object(row_object->data[1]) == 90);
    check_malformed_entry([&] {
      static_cast<void>(rt::convert<table_type>::from_object(scalar_entry_vector.erase()));
    });
    check_malformed_entry([&] {
      static_cast<void>(
        rt::convert<table_transient_type>::from_object(scalar_entry_vector.erase()));
    });
  }
}
