#include <jank/hash.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::hash
{
  static constexpr uint32_t seed{ 0 };
  static constexpr uint32_t C1{ 0xcc9e2d51 };
  static constexpr uint32_t C2{ 0x1b873593 };

  uint32_t rotate_left(uint32_t const x, int8_t const r)
  {
    return (x << r) | (x >> (32 - r));
  }

  uint64_t rotate_left(uint64_t const x, int8_t const r)
  {
    return (x << r) | (x >> (64 - r));
  }

  uint32_t fmix(uint32_t h)
  {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
  }

  uint32_t fmix(uint32_t h1, uint32_t const length)
  {
    h1 ^= length;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
  }

  uint32_t mix_k1(uint32_t k1)
  {
    k1 *= C1;
    k1 = rotate_left(k1, 15);
    k1 *= C2;
    return k1;
  }

  uint32_t mix_h1(uint32_t h1, uint32_t k1)
  {
    h1 ^= k1;
    h1 = rotate_left(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
    return h1;
  }

  uint32_t mix_collection_hash(uint32_t const hash, uint32_t const length)
  {
    uint32_t const k1{ mix_k1(hash) };
    uint32_t const h1{ mix_h1(seed, k1) };
    return fmix(h1, length);
  }

  uint32_t combine(uint32_t const seed, uint32_t const t)
  {
    return seed ^ (t + 0x9e3779b9 + (seed << 6) + (seed >> 2));
  }

  uint32_t integer(uint32_t const input)
  {
    if(input == 0)
    {
      return 0;
    }

    auto const k1(mix_k1(input));
    auto const h1(mix_h1(seed, k1));

    return fmix(h1, 4);
  }

  uint32_t integer(uint64_t const input)
  {
    if(input == 0)
    {
      return 0;
    }

    auto const low(static_cast<uint32_t>(input));
    auto const high(static_cast<uint32_t>(input >> 32));

    auto k1(mix_k1(low));
    auto h1(mix_h1(seed, k1));

    k1 = mix_k1(high);
    h1 = mix_h1(h1, k1);

    return fmix(h1, 8);
  }

  uint32_t integer(native_integer const input)
  {
    if constexpr(sizeof(uint64_t) == sizeof(native_integer))
    {
      return integer(static_cast<uint64_t>(input));
    }
    else
    {
      return integer(static_cast<uint32_t>(input));
    }
  }

  uint32_t real(native_real const input)
  {
    if constexpr(8 == sizeof(native_integer))
    {
      auto const v(*reinterpret_cast<uint64_t const *>(&input));
      return v ^ (v >> 32);
    }
    else
    {
      return *reinterpret_cast<uint32_t const *>(&input);
    }
  }

  uint32_t string(native_persistent_string_view const &input)
  {
    auto const length(input.size());
    uint32_t h1{ seed };

    for(size_t i{ 1 }; i < length; i += 2)
    {
      auto k1(static_cast<uint32_t>(input[i - 1] | (input[i] << 16)));
      k1 = mix_k1(k1);
      h1 = mix_h1(h1, k1);
    }

    if((length & 1) == 1)
    {
      auto k1(static_cast<uint32_t>(static_cast<uint8_t>(input[length - 1])));
      k1 = mix_k1(k1);
      h1 ^= k1;
    }

    return fmix(h1, 2 * length);
  }

  uint32_t visit(char const ch)
  {
    return static_cast<uint32_t>(ch);
  }

  uint32_t visit(runtime::object const * const o)
  {
    assert(o);
    return runtime::visit_object([](auto const typed_o) { return typed_o->to_hash(); }, o);
  }

  uint32_t ordered(runtime::object const * const sequence)
  {
    assert(sequence);
    return runtime::visit_object(
      [](auto const typed_sequence) -> uint32_t {
        using T = typename decltype(typed_sequence)::value_type;
        if constexpr(runtime::behavior::sequenceable<T>)
        {
          uint32_t n{};
          uint32_t hash{ 1 };
          for(auto it(fresh_seq(typed_sequence)); it != nullptr; it = next_in_place(it))
          {
            hash = 31 * hash + visit(it->first());
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

  uint32_t unordered(runtime::object const * const sequence)
  {
    assert(sequence);
    return runtime::visit_object(
      [](auto const typed_sequence) -> uint32_t {
        using T = typename decltype(typed_sequence)::value_type;
        if constexpr(runtime::behavior::sequenceable<T>)
        {
          uint32_t n{};
          uint32_t hash{ 1 };
          for(auto it(fresh_seq(typed_sequence)); it != nullptr; it = next_in_place(it))
          {
            hash += visit(it->first());
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
