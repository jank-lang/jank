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
  u32 rotate_left(u32 const x, i8 const r);
  u64 rotate_left(u64 const x, i8 const r);

  u32 fmix(u32 h);
  u32 fmix(u32 h1, u32 const length);

  u32 mix_k1(u32 k1);
  u32 mix_h1(u32 h1, u32 k1);

  u32 mix_collection_hash(u32 const hash, u32 const length);

  u32 combine(u32 const seed, u32 const t);

  u32 integer(u32 const input);
  u32 integer(u64 const input);
  u32 integer(i64 const input);

  u32 real(f64 const input);

  u32 string(native_persistent_string_view const &input);

  u32 visit(runtime::oref<runtime::object> const o);
  u32 visit(char const ch);

  u32 ordered(runtime::object const * const sequence);
  u32 unordered(runtime::object const * const sequence);

  template <typename It>
  u32 ordered(It const &begin, It const &end)
  {
    u32 n{};
    u32 hash{ 1 };

    for(auto it(begin); it != end; ++it)
    {
      hash = 31 * hash + visit((*it));
      ++n;
    }

    return mix_collection_hash(hash, n);
  }

  template <typename It>
  u32 unordered(It const &begin, It const &end)
  {
    using T = typename std::iterator_traits<It>::value_type;

    u32 hash{};
    u32 n{};

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
