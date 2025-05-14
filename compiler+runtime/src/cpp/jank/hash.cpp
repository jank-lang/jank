#include <jank/hash.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/sequence_range.hpp>

namespace jank::hash
{
  static constexpr u32 seed{ 0 };
  static constexpr u32 C1{ 0xcc9e2d51 };
  static constexpr u32 C2{ 0x1b873593 };

  u32 rotate_left(u32 const x, i8 const r)
  {
    return (x << r) | (x >> (32 - r));
  }

  u64 rotate_left(u64 const x, i8 const r)
  {
    return (x << r) | (x >> (64 - r));
  }

  u32 fmix(u32 h)
  {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
  }

  u32 fmix(u32 h1, u32 const length)
  {
    h1 ^= length;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
  }

  u32 mix_k1(u32 k1)
  {
    k1 *= C1;
    k1 = rotate_left(k1, 15);
    k1 *= C2;
    return k1;
  }

  u32 mix_h1(u32 h1, u32 k1)
  {
    h1 ^= k1;
    h1 = rotate_left(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
    return h1;
  }

  u32 mix_collection_hash(u32 const hash, u32 const length)
  {
    u32 const k1{ mix_k1(hash) };
    u32 const h1{ mix_h1(seed, k1) };
    return fmix(h1, length);
  }

  u32 combine(u32 const seed, u32 const t)
  {
    return seed ^ (t + 0x9e3779b9 + (seed << 6) + (seed >> 2));
  }

  u32 integer(u32 const input)
  {
    if(input == 0)
    {
      return 0;
    }

    auto const k1(mix_k1(input));
    auto const h1(mix_h1(seed, k1));

    return fmix(h1, 4);
  }

  u32 integer(u64 const input)
  {
    if(input == 0)
    {
      return 0;
    }

    auto const low(static_cast<u32>(input));
    auto const high(static_cast<u32>(input >> 32));

    auto k1(mix_k1(low));
    auto h1(mix_h1(seed, k1));

    k1 = mix_k1(high);
    h1 = mix_h1(h1, k1);

    return fmix(h1, 8);
  }

  u32 integer(i64 const input)
  {
    if constexpr(sizeof(u64) == sizeof(i64))
    {
      return integer(static_cast<u64>(input));
    }
    else
    {
      return integer(static_cast<u32>(input));
    }
  }

  u32 real(f64 const input)
  {
    if constexpr(8 == sizeof(i64))
    {
      auto const v(*reinterpret_cast<u64 const *>(&input));
      return v ^ (v >> 32);
    }
    else
    {
      return *reinterpret_cast<u32 const *>(&input);
    }
  }

  u32 string(native_persistent_string_view const &input)
  {
    auto const length(input.size());
    u32 h1{ seed };

    for(usize i{ 1 }; i < length; i += 2)
    {
      auto k1(static_cast<u32>(input[i - 1] | (input[i] << 16)));
      k1 = mix_k1(k1);
      h1 = mix_h1(h1, k1);
    }

    if((length & 1) == 1)
    {
      auto k1(static_cast<u32>(static_cast<u8>(input[length - 1])));
      k1 = mix_k1(k1);
      h1 ^= k1;
    }

    return fmix(h1, 2 * length);
  }

  u32 visit(char const ch)
  {
    return static_cast<u32>(ch);
  }

  u32 visit(runtime::object_ref const o)
  {
    return runtime::visit_object([](auto const typed_o) -> u32 { return typed_o->to_hash(); }, o);
  }

  template <typename T>
  requires runtime::behavior::object_like<T>
  static u32 visit(runtime::oref<T> const o)
  {
    return o->to_hash();
  }

  u32 ordered(runtime::object const * const sequence)
  {
    jank_debug_assert(sequence);
    return runtime::visit_object(
      [](auto const typed_sequence) -> u32 {
        using T = typename decltype(typed_sequence)::value_type;
        if constexpr(runtime::behavior::sequenceable<T>)
        {
          u32 n{};
          u32 hash{ 1 };
          for(auto const e : make_sequence_range(typed_sequence))
          {
            hash = 31 * hash + visit(e);
            ++n;
          }

          return mix_collection_hash(hash, n);
        }
        else
        {
          return typed_sequence->to_hash();
        }
      },
      sequence);
  }

  u32 unordered(runtime::object const * const sequence)
  {
    jank_debug_assert(sequence);
    return runtime::visit_object(
      [](auto const typed_sequence) -> u32 {
        using T = typename decltype(typed_sequence)::value_type;
        if constexpr(runtime::behavior::sequenceable<T>)
        {
          u32 n{};
          u32 hash{ 1 };
          for(auto const e : make_sequence_range(typed_sequence))
          {
            hash += visit(e);
            ++n;
          }

          return mix_collection_hash(hash, n);
        }
        else
        {
          return typed_sequence->to_hash();
        }
      },
      sequence);
  }
}
