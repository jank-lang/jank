#pragma once

#include <cstddef>
#include <utility>

namespace new_model
{
  enum behavior_type
  {
    behavior_type_none = 0,
    behavior_type_nil = 1,
    behavior_type_integer,
    behavior_type_real,
    behavior_type_number,

    behavior_type_countable,
    behavior_type_seqable,
    behavior_type_metadatable,
    behavior_type_associatively_readable,
    behavior_type_associatively_writable,

    behavior_type_map = behavior_type_countable | behavior_type_seqable | behavior_type_metadatable | behavior_type_associatively_readable | behavior_type_associatively_writable
  };

  enum storage_type
  {
    storage_type_none = 0,
    storage_type_min_bit = 1,
    storage_type_max_bit = 3,
    storage_type_integer = (1 << 0),
    storage_type_real = (1 << 1),
    storage_type_map = (1 << 2),
    storage_type_metadatable = (1 << 3)
  };

  template <typename Head, typename Tail>
  struct type_cons;

  template <typename Head, typename ...Tail>
  struct type_cons<Head, std::tuple<Tail...>>
  { using type = std::tuple<Head, Tail...>; };

  template <typename Pred, typename...>
  struct filter;

  template <typename Pred>
  struct filter<Pred>
  { using type = std::tuple<>; };

  template <typename Pred, typename Head, typename ...Tail>
  struct filter<Pred, Head, Tail...>
  {
    using type = std::conditional_t
    <
      Pred::template apply<Head>::value,
      typename type_cons<typename Head::type, typename filter<Pred, Tail...>::type>::type,
      typename filter<Pred, Tail...>::type
    >;
  };

  template <int S>
  struct is_storage_enabled
  {
    template <typename T>
    struct apply
    { static constexpr bool const value{ (S & T::value) == T::value }; };
  };

  template <int S>
  struct storage_base;

  template <>
  struct storage_base<storage_type_integer>
  {
    using type = int64_t;
    static constexpr int const value{ storage_type_integer };
  };

  template <>
  struct storage_base<storage_type_real>
  {
    using type = long double;
    static constexpr int const value{ storage_type_real };
  };

  template <>
  struct storage_base<storage_type_map>
  {
    using type = void*;
    static constexpr int const value{ storage_type_map };
  };

  template <>
  struct storage_base<storage_type_metadatable>
  {
    using type = void*;
    static constexpr int const value{ storage_type_metadatable };
  };

  template <int Min, typename Ints>
  struct storage_base_tuple_impl;

  template <int Min>
  struct storage_base_tuple_impl<Min, std::integer_sequence<int>>
  { using type = std::tuple<>; };

  template <int Min, int S, int ...Ints>
  struct storage_base_tuple_impl<Min, std::integer_sequence<int, S, Ints...>>
  {
    using type = typename type_cons
    <
      storage_base<(1 << (Min + S))>,
      typename storage_base_tuple_impl<Min, std::integer_sequence<int, Ints...>>::type
    >::type;
  };

  template <int Min, int Max>
  struct storage_base_tuple
  {
    using type = typename storage_base_tuple_impl<Min, std::make_integer_sequence<int, Max - Min>>::type;
  };

  struct object
  {
    behavior_type behavior{};
    storage_type storage{};
  };

  template <int B, int S>
  struct typed_object : gc
  {
    object base;

    typename filter
    <
      is_storage_enabled<S>,
      storage_base<storage_type_integer>,
      storage_base<storage_type_real>,
      storage_base<storage_type_map>,
      storage_base<storage_type_metadatable>
    >::type data{};

    typename filter
    <
      is_storage_enabled<S>,
      storage_base_tuple<storage_type_min_bit, storage_type_max_bit>::type
    >::type _data{};
  };

  template <int B, int S>
  auto make_object()
  {
    using T = typed_object<B, S>;
    static_assert(offsetof(T, base) == 0);
    return new (GC) typed_object<B, S>{ {}, { static_cast<behavior_type>(B), static_cast<storage_type>(S) }, {} };
  }
}
