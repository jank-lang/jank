#pragma once

#include <type_traits>

#include <jtl/ref.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::detail
{
  /* TODO: Move this somewhere more general. It's used by other collections. */
  struct in_place_unique
  {
  };

  /* This is a short map, storing a vector of pairs. This is only until immer has proper
   * support for short maps and map transients. */
  struct native_persistent_array_map
  {
    /* Array maps are fast only for a small number of keys. Clojure JVM uses a threshold of 8
     * k/v pairs, thus 16 elements. We follow the same. */
    /* TODO: Benchmark difference thresholds. */
    static constexpr usize max_size{ 8 };

    native_persistent_array_map() = default;
    native_persistent_array_map(native_persistent_array_map const &s) = default;
    native_persistent_array_map(native_persistent_array_map &&s) noexcept = default;

    template <typename L, typename E = std::enable_if_t<std::is_integral_v<L>>>
    native_persistent_array_map(in_place_unique, jtl::ref<object_ref> const kvs, L const l)
      : data{ std::move(kvs.data) }
      , length{ static_cast<decltype(length)>(l) }
    {
    }

    ~native_persistent_array_map() = default;

    void insert_unique(object_ref const key, object_ref const val);

    void insert_or_assign(object_ref const key, object_ref const val);

    void erase(object_ref const key);

    object_ref find(object_ref const key) const;

    native_hash to_hash() const;

    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = std::pair<object_ref, object_ref>;
      using pointer = value_type *;
      using reference = value_type &;

      iterator(object_ref const *data, usize index);
      iterator(iterator const &) = default;
      iterator(iterator &&) noexcept = default;

      value_type operator*() const;

      iterator &operator++();

      bool operator!=(iterator const &rhs) const;

      bool operator==(iterator const &rhs) const;

      iterator &operator=(iterator const &rhs);

      object_ref const *data{};
      usize index{};
    };

    using const_iterator = iterator;

    const_iterator begin() const;
    const_iterator end() const;

    usize size() const;

    bool empty() const;

    native_persistent_array_map clone() const;

    object_ref *data{};
    usize length{};
    mutable native_hash hash{};
  };
}
