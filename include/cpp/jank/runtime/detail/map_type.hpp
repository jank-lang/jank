#pragma once

#include <ostream>
#include <memory>

#include <jank/runtime/hash.hpp>

namespace jank::runtime::detail
{
  struct in_place_unique
  { };

  /* This is a short map, storing a vector of pairs. This is only until immer has proper
   * support for short maps and map transients. */
  template <typename K, typename V>
  struct map_type_impl
  {
    using value_type = native_vector<std::pair<K, V>>;
    using iterator = typename value_type::iterator;
    using const_iterator = typename value_type::const_iterator;

    map_type_impl() = default;
    map_type_impl(map_type_impl const &s) = default;
    map_type_impl(map_type_impl &&s) noexcept = default;
    map_type_impl(in_place_unique, value_type &&kvs)
      : data{ std::move(kvs) }
    { }
    ~map_type_impl() = default;

    template <typename NK, typename NV>
    void insert_or_assign(NK &&key, NV &&val)
    {
      if(auto * const found = find_ref(key))
      { *found = std::forward<NV>(val); }
      else
      { data.emplace_back(std::forward<NK>(key), std::forward<NV>(val)); }
    }
    template <typename NK, typename NV>
    void insert_unique(NK &&key, NV &&val)
    { data.emplace_back(std::forward<NK>(key), std::forward<NV>(val)); }

    V find(K const &key) const
    {
      if(auto const kw = key->as_keyword())
      {
        for(auto const &kv : data)
        {
          if(kv.first == key)
          { return kv.second; }
        }
      }
      else
      {
        for(auto const &kv : data)
        {
          if(kv.first->equal(*key))
          { return kv.second; }
        }
      }
      return nullptr;
    }

    V* find_ref(K const &key)
    {
      if(auto const kw = key->as_keyword())
      {
        for(auto &kv : data)
        {
          if(kv.first == key)
          { return &kv.second; }
        }
      }
      else
      {
        for(auto &kv : data)
        {
          if(kv.first->equal(*key))
          { return &kv.second; }
        }
      }
      return nullptr;
    }

    size_t to_hash() const
    {
      if(hash != 0)
      { return hash; }

      hash = data.size();
      for(auto const &kv : data)
      {
        hash = jank::runtime::detail::hash_combine(hash, *kv.first);
        hash = jank::runtime::detail::hash_combine(hash, *kv.second);
      }
      return hash;
    }

    iterator begin()
    { return data.begin(); }
    const_iterator begin() const
    { return data.begin(); }
    iterator end()
    { return data.end(); }
    const_iterator end() const
    { return data.end(); }

    size_t size() const
    { return data.size(); }

    value_type data;
    mutable size_t hash{};
  };
}
