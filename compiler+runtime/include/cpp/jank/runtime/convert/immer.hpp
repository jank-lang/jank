#pragma once

#include <jank/runtime/convert/builtin.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/visit.hpp>
#include <immer/array.hpp>
#include <immer/array_transient.hpp>
#include <immer/box.hpp>
#include <immer/experimental/dvektor.hpp>
#include <immer/flex_vector.hpp>
#include <immer/flex_vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/table.hpp>
#include <immer/table_transient.hpp>
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>
#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>

#include <concepts>
#include <stdexcept>
#include <utility>

namespace jank::runtime
{
  namespace detail
  {
    template <typename T, typename Insertable, typename Seqable>
    Insertable convert_seqable_to_insertable(oref<Seqable> const o)
    {
      auto values(collect_seqable_values<T>(o));

      Insertable ret;
      for(auto &value : values)
      {
        ret.insert(std::move(value));
      }
      return ret;
    }

    template <typename T, typename PushBackable, typename Seqable>
    PushBackable convert_seqable_to_persistent_push_backable(oref<Seqable> const o)
    {
      auto values(collect_seqable_values<T>(o));

      PushBackable ret;
      for(auto &value : values)
      {
        ret = ret.push_back(std::move(value));
      }
      return ret;
    }

    template <typename K, typename V>
    std::pair<K, V> convert_entry_to_pair(object_ref const entry)
    {
      auto const invalid_entry([]() -> std::pair<K, V> {
        throw std::runtime_error{ "map entry must contain exactly two values" };
      });

      return visit_seqable(
        [](auto const typed_entry) -> std::pair<K, V> {
          native_vector<object_ref> objects;
          for(auto const o : make_sequence_range(typed_entry))
          {
            objects.push_back(o);
            if(objects.size() > 2)
            {
              throw std::runtime_error{ "map entry must contain exactly two values" };
            }
          }

          if(objects.size() != 2)
          {
            throw std::runtime_error{ "map entry must contain exactly two values" };
          }

          auto key(convert<K>::from_object(objects[0]));
          auto value(convert<V>::from_object(objects[1]));
          return { std::move(key), std::move(value) };
        },
        invalid_entry,
        entry);
    }

    template <typename K, typename V, typename Seqable>
    native_vector<std::pair<K, V>> collect_entry_seqable_pairs(oref<Seqable> const o)
    {
      native_vector<std::pair<K, V>> ret;
      for(auto const entry : make_sequence_range(o))
      {
        ret.emplace_back(convert_entry_to_pair<K, V>(entry));
      }
      return ret;
    }

    template <typename K, typename V, typename Associative, typename Seqable>
    Associative convert_entry_seqable_to_associative(oref<Seqable> const o)
    {
      auto pairs(collect_entry_seqable_pairs<K, V>(o));

      Associative ret;
      for(auto &pair : pairs)
      {
        ret.set(std::move(pair.first), std::move(pair.second));
      }
      return ret;
    }

    template <typename K, typename V, typename MapLike>
    native_vector<std::pair<K, V>> collect_map_like_pairs(oref<MapLike> const o)
    {
      native_vector<std::pair<K, V>> ret;
      for(auto const &[k, v] : o->data)
      {
        ret.emplace_back(convert<K>::from_object(k), convert<V>::from_object(v));
      }
      return ret;
    }

    template <typename K, typename V, typename Associative, typename MapLike>
    Associative convert_map_like_to_associative(oref<MapLike> const o)
    {
      auto pairs(collect_map_like_pairs<K, V>(o));

      Associative ret;
      for(auto &pair : pairs)
      {
        ret.set(std::move(pair.first), std::move(pair.second));
      }
      return ret;
    }

    template <typename T, typename KeyFn, typename Key, typename Value>
    T make_table_row(Key &&key, Value &&value)
    {
      return KeyFn{}(std::forward<Value>(value), std::forward<Key>(key));
    }

    template <typename T, typename KeyFn, typename TableLike, typename Seqable>
    native_vector<T> collect_entry_seqable_table_rows(oref<Seqable> const o)
    {
      using key_type = typename TableLike::key_type;

      native_vector<T> ret;
      for(auto const entry : make_sequence_range(o))
      {
        auto pair(convert_entry_to_pair<key_type, T>(entry));
        ret.emplace_back(make_table_row<T, KeyFn>(std::move(pair.first), std::move(pair.second)));
      }
      return ret;
    }

    template <typename T, typename KeyFn, typename TableLike, typename Seqable>
    TableLike convert_entry_seqable_to_table(oref<Seqable> const o)
    {
      auto rows(collect_entry_seqable_table_rows<T, KeyFn, TableLike>(o));

      TableLike ret;
      for(auto &row : rows)
      {
        ret.insert(std::move(row));
      }
      return ret;
    }

    template <typename T, typename KeyFn, typename TableLike, typename MapLike>
    native_vector<T> collect_map_like_table_rows(oref<MapLike> const o)
    {
      using key_type = typename TableLike::key_type;

      native_vector<T> ret;
      for(auto const &[k, v] : o->data)
      {
        auto key(convert<key_type>::from_object(k));
        auto value(convert<T>::from_object(v));
        ret.emplace_back(make_table_row<T, KeyFn>(std::move(key), std::move(value)));
      }
      return ret;
    }

    template <typename T, typename KeyFn, typename TableLike, typename MapLike>
    TableLike convert_map_like_to_table(oref<MapLike> const o)
    {
      auto rows(collect_map_like_table_rows<T, KeyFn, TableLike>(o));

      TableLike ret;
      for(auto &row : rows)
      {
        ret.insert(std::move(row));
      }
      return ret;
    }
  }

  /* Support for jank-policy immer vectors, which are the safe persistent vector shape for
   * runtime/JIT interop. */
  template <typename T, immer::detail::rbts::bits_t B, immer::detail::rbts::bits_t BL>
  requires(convertible<T>)
  struct convert<immer::vector<T, ::jank::memory_policy, B, BL>>
  {
    using vector_type = immer::vector<T, ::jank::memory_policy, B, BL>;

    static obj::persistent_vector_ref into_object(vector_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static vector_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static vector_type from_object(oref<Seqable> const o)
    {
      auto trans(
        detail::convert_seqable_to_push_backable<T, typename vector_type::transient_type>(o));
      return trans.persistent();
    }
  };

  /* Support for jank-policy immer vector transients, sharing the persistent-vector boundary. */
  template <typename T, immer::detail::rbts::bits_t B, immer::detail::rbts::bits_t BL>
  requires(convertible<T>)
  struct convert<immer::vector_transient<T, ::jank::memory_policy, B, BL>>
  {
    using vector_type = immer::vector_transient<T, ::jank::memory_policy, B, BL>;

    static obj::persistent_vector_ref into_object(vector_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static vector_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static vector_type from_object(oref<Seqable> const o)
    {
      return detail::convert_seqable_to_push_backable<T, vector_type>(o);
    }
  };

  /* Support for jank-policy immer flex vectors, sharing the persistent-vector boundary. */
  template <typename T, immer::detail::rbts::bits_t B, immer::detail::rbts::bits_t BL>
  requires(convertible<T>)
  struct convert<immer::flex_vector<T, ::jank::memory_policy, B, BL>>
  {
    using vector_type = immer::flex_vector<T, ::jank::memory_policy, B, BL>;

    static obj::persistent_vector_ref into_object(vector_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static vector_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static vector_type from_object(oref<Seqable> const o)
    {
      auto trans(
        detail::convert_seqable_to_push_backable<T, typename vector_type::transient_type>(o));
      return trans.persistent();
    }
  };

  /* Support for jank-policy immer flex-vector transients, sharing the persistent-vector boundary. */
  template <typename T, immer::detail::rbts::bits_t B, immer::detail::rbts::bits_t BL>
  requires(convertible<T>)
  struct convert<immer::flex_vector_transient<T, ::jank::memory_policy, B, BL>>
  {
    using vector_type = immer::flex_vector_transient<T, ::jank::memory_policy, B, BL>;

    static obj::persistent_vector_ref into_object(vector_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static vector_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static vector_type from_object(oref<Seqable> const o)
    {
      return detail::convert_seqable_to_push_backable<T, vector_type>(o);
    }
  };

  /* Support for jank-policy experimental immer dvektors, sharing the persistent-vector
   * boundary. */
  template <typename T, int B>
  requires(convertible<T>)
  struct convert<immer::dvektor<T, B, ::jank::memory_policy>>
  {
    using vector_type = immer::dvektor<T, B, ::jank::memory_policy>;

    static obj::persistent_vector_ref into_object(vector_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static vector_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static vector_type from_object(oref<Seqable> const o)
    {
      return detail::convert_seqable_to_persistent_push_backable<T, vector_type>(o);
    }
  };

  /* Support for jank-policy immer arrays, sharing the persistent-vector boundary. */
  template <typename T>
  requires(convertible<T>)
  struct convert<immer::array<T, ::jank::memory_policy>>
  {
    using array_type = immer::array<T, ::jank::memory_policy>;

    static obj::persistent_vector_ref into_object(array_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static array_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static array_type from_object(oref<Seqable> const o)
    {
      auto trans(
        detail::convert_seqable_to_push_backable<T, typename array_type::transient_type>(o));
      return trans.persistent();
    }
  };

  /* Support for jank-policy immer array transients, sharing the persistent-vector boundary. */
  template <typename T>
  requires(convertible<T>)
  struct convert<immer::array_transient<T, ::jank::memory_policy>>
  {
    using array_type = immer::array_transient<T, ::jank::memory_policy>;

    static obj::persistent_vector_ref into_object(array_type const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_vector>(trans.persistent());
    }

    static array_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static array_type from_object(oref<Seqable> const o)
    {
      return detail::convert_seqable_to_push_backable<T, array_type>(o);
    }
  };

  /* Support for jank-policy immer boxes, using the boxed value at the runtime boundary. */
  template <typename T>
  requires(convertible<T>)
  struct convert<immer::box<T, ::jank::memory_policy>>
  {
    using box_type = immer::box<T, ::jank::memory_policy>;

    static auto into_object(box_type const &o)
    {
      return convert<T>::into_object(o.get());
    }

    static box_type from_object(object_ref const o)
    {
      return box_type{ convert<T>::from_object(o) };
    }
  };

  /* Support for jank-policy immer maps, matching jank's runtime persistent maps. */
  template <typename K, typename V, typename Hash, typename Equal, immer::detail::hamts::bits_t B>
  requires(convertible<K> && convertible<V>)
  struct convert<immer::map<K, V, Hash, Equal, ::jank::memory_policy, B>>
  {
    using map_type = immer::map<K, V, Hash, Equal, ::jank::memory_policy, B>;

    static obj::persistent_hash_map_ref into_object(map_type const &o)
    {
      runtime::detail::native_transient_hash_map trans;
      for(auto const &[k, v] : o)
      {
        trans.set(convert<K>::into_object(k), convert<V>::into_object(v));
      }
      return make_box<obj::persistent_hash_map>(trans.persistent());
    }

    static map_type from_object(object_ref const o)
    {
      return visit_map_like(
        [](auto const typed_o) { return from_map_like(typed_o); },
        [=]() {
          return visit_seqable([](auto const typed_o) { return from_entries(typed_o); }, o);
        },
        o);
    }

    template <typename MapLike>
    static map_type from_map_like(oref<MapLike> const o)
    {
      auto trans(
        detail::convert_map_like_to_associative<K, V, typename map_type::transient_type>(o));
      return trans.persistent();
    }

    template <typename Seqable>
    static map_type from_entries(oref<Seqable> const o)
    {
      auto trans(
        detail::convert_entry_seqable_to_associative<K, V, typename map_type::transient_type>(o));
      return trans.persistent();
    }
  };

  /* Support for jank-policy immer map transients, matching jank's runtime persistent maps. */
  template <typename K, typename V, typename Hash, typename Equal, immer::detail::hamts::bits_t B>
  requires(convertible<K> && convertible<V>)
  struct convert<immer::map_transient<K, V, Hash, Equal, ::jank::memory_policy, B>>
  {
    using map_type = immer::map_transient<K, V, Hash, Equal, ::jank::memory_policy, B>;

    static obj::persistent_hash_map_ref into_object(map_type const &o)
    {
      runtime::detail::native_transient_hash_map trans;
      for(auto const &[k, v] : o)
      {
        trans.set(convert<K>::into_object(k), convert<V>::into_object(v));
      }
      return make_box<obj::persistent_hash_map>(trans.persistent());
    }

    static map_type from_object(object_ref const o)
    {
      return visit_map_like(
        [](auto const typed_o) { return from_map_like(typed_o); },
        [=]() {
          return visit_seqable([](auto const typed_o) { return from_entries(typed_o); }, o);
        },
        o);
    }

    template <typename MapLike>
    static map_type from_map_like(oref<MapLike> const o)
    {
      return detail::convert_map_like_to_associative<K, V, map_type>(o);
    }

    template <typename Seqable>
    static map_type from_entries(oref<Seqable> const o)
    {
      return detail::convert_entry_seqable_to_associative<K, V, map_type>(o);
    }
  };

  /* Support for jank-policy immer sets, matching jank's runtime persistent sets. */
  template <typename T, typename Hash, typename Equal, immer::detail::hamts::bits_t B>
  requires(convertible<T>)
  struct convert<immer::set<T, Hash, Equal, ::jank::memory_policy, B>>
  {
    using set_type = immer::set<T, Hash, Equal, ::jank::memory_policy, B>;

    static obj::persistent_hash_set_ref into_object(set_type const &o)
    {
      runtime::detail::native_transient_hash_set trans;
      for(auto const &e : o)
      {
        trans.insert(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_hash_set>(trans.persistent());
    }

    static set_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static set_type from_object(oref<Seqable> const o)
    {
      auto trans(detail::convert_seqable_to_insertable<T, typename set_type::transient_type>(o));
      return trans.persistent();
    }
  };

  /* Support for jank-policy immer set transients, matching jank's runtime persistent sets. */
  template <typename T, typename Hash, typename Equal, immer::detail::hamts::bits_t B>
  requires(convertible<T>)
  struct convert<immer::set_transient<T, Hash, Equal, ::jank::memory_policy, B>>
  {
    using set_type = immer::set_transient<T, Hash, Equal, ::jank::memory_policy, B>;

    static obj::persistent_hash_set_ref into_object(set_type const &o)
    {
      runtime::detail::native_transient_hash_set trans;
      for(auto const &e : o)
      {
        trans.insert(convert<T>::into_object(e));
      }
      return make_box<obj::persistent_hash_set>(trans.persistent());
    }

    static set_type from_object(object_ref const o)
    {
      return visit_seqable([](auto const typed_o) { return from_object(typed_o); }, o);
    }

    template <typename Seqable>
    static set_type from_object(oref<Seqable> const o)
    {
      return detail::convert_seqable_to_insertable<T, set_type>(o);
    }
  };

  /* Support for jank-policy immer tables, exposing table keys as jank map keys. */
  template <typename T,
            typename KeyFn,
            typename Hash,
            typename Equal,
            immer::detail::hamts::bits_t B>
  requires(convertible<T> && convertible<immer::table_key_t<KeyFn, T>>
           && requires(T value, immer::table_key_t<KeyFn, T> key) {
                { KeyFn{}(std::move(value), std::move(key)) } -> std::convertible_to<T>;
              })
  struct convert<immer::table<T, KeyFn, Hash, Equal, ::jank::memory_policy, B>>
  {
    using table_type = immer::table<T, KeyFn, Hash, Equal, ::jank::memory_policy, B>;
    using key_type = typename table_type::key_type;

    static obj::persistent_hash_map_ref into_object(table_type const &o)
    {
      runtime::detail::native_transient_hash_map trans;
      for(auto const &e : o)
      {
        trans.set(convert<key_type>::into_object(KeyFn{}(e)), convert<T>::into_object(e));
      }
      return make_box<obj::persistent_hash_map>(trans.persistent());
    }

    static table_type from_object(object_ref const o)
    {
      return visit_map_like(
        [](auto const typed_o) { return from_map_like(typed_o); },
        [=]() {
          return visit_seqable([](auto const typed_o) { return from_entries(typed_o); }, o);
        },
        o);
    }

    template <typename MapLike>
    static table_type from_map_like(oref<MapLike> const o)
    {
      auto trans(
        detail::convert_map_like_to_table<T, KeyFn, typename table_type::transient_type>(o));
      return trans.persistent();
    }

    template <typename Seqable>
    static table_type from_entries(oref<Seqable> const o)
    {
      auto trans(
        detail::convert_entry_seqable_to_table<T, KeyFn, typename table_type::transient_type>(o));
      return trans.persistent();
    }
  };

  /* Support for jank-policy immer table transients, exposing table keys as jank map keys. */
  template <typename T,
            typename KeyFn,
            typename Hash,
            typename Equal,
            immer::detail::hamts::bits_t B>
  requires(convertible<T> && convertible<immer::table_key_t<KeyFn, T>>
           && requires(T value, immer::table_key_t<KeyFn, T> key) {
                { KeyFn{}(std::move(value), std::move(key)) } -> std::convertible_to<T>;
              })
  struct convert<immer::table_transient<T, KeyFn, Hash, Equal, ::jank::memory_policy, B>>
  {
    using table_type = immer::table_transient<T, KeyFn, Hash, Equal, ::jank::memory_policy, B>;
    using key_type = typename table_type::key_type;

    static obj::persistent_hash_map_ref into_object(table_type const &o)
    {
      runtime::detail::native_transient_hash_map trans;
      for(auto const &e : o)
      {
        trans.set(convert<key_type>::into_object(KeyFn{}(e)), convert<T>::into_object(e));
      }
      return make_box<obj::persistent_hash_map>(trans.persistent());
    }

    static table_type from_object(object_ref const o)
    {
      return visit_map_like(
        [](auto const typed_o) { return from_map_like(typed_o); },
        [=]() {
          return visit_seqable([](auto const typed_o) { return from_entries(typed_o); }, o);
        },
        o);
    }

    template <typename MapLike>
    static table_type from_map_like(oref<MapLike> const o)
    {
      return detail::convert_map_like_to_table<T, KeyFn, table_type>(o);
    }

    template <typename Seqable>
    static table_type from_entries(oref<Seqable> const o)
    {
      return detail::convert_entry_seqable_to_table<T, KeyFn, table_type>(o);
    }
  };
}
