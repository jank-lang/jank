#pragma once

#include <cstdint>

#include <jank/type.hpp>

namespace jank::runtime
{
  struct object;

  template <typename T>
  struct oref;
}

namespace jank::hash
{
  uint32_t rotate_left(uint32_t const x, int8_t const r);
  uint64_t rotate_left(uint64_t const x, int8_t const r);

  uint32_t fmix(uint32_t h);
  uint32_t fmix(uint32_t h1, uint32_t const length);

  uint32_t mix_k1(uint32_t k1);
  uint32_t mix_h1(uint32_t h1, uint32_t k1);

  uint32_t mix_collection_hash(uint32_t const hash, uint32_t const length);

  uint32_t combine(uint32_t const seed, uint32_t const t);

  uint32_t integer(uint32_t const input);
  uint32_t integer(uint64_t const input);
  uint32_t integer(native_integer const input);

  uint32_t real(native_real const input);

  uint32_t string(native_persistent_string_view const &input);

  uint32_t visit(runtime::oref<runtime::object> const o);
  uint32_t visit(char const ch);

  uint32_t ordered(runtime::object const * const sequence);
  uint32_t unordered(runtime::object const * const sequence);

  template <typename It>
  uint32_t ordered(It const &begin, It const &end)
  {
    uint32_t n{};
    uint32_t hash{ 1 };

    for(auto it(begin); it != end; ++it)
    {
      hash = 31 * hash + visit((*it));
      ++n;
    }

    return mix_collection_hash(hash, n);
  }

  template <typename It>
  uint32_t unordered(It const &begin, It const &end)
  {
    using T = typename std::iterator_traits<It>::value_type;

    uint32_t hash{};
    uint32_t n{};

    for(auto it(begin); it != end; ++it)
    {
      /* It's common that we have pairs of data, like with maps. */
      if constexpr(requires(T t) { t.first, t.second; })
      {
        hash += 31 * visit((*it).first) + visit((*it).second);
      }
      else
      {
        hash += visit((*it).get());
      }
      ++n;
    }

    return mix_collection_hash(hash, n);
  }
}
