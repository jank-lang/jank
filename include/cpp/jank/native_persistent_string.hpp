#pragma once

#include <bit>
#include <fmt/format.h>

namespace jank
{
  /* This is a not-completely-standard replacement for std::string, with a few goals in mind:
   *
   * 1. Be as fast, or faster, than `std::string` and `folly::fbstring`
   * 2. Support hashing, with cached value
   * 3. Be immutable (i.e. no copy on substrings, writes only done in constructors, no mutators)
   * 4. Not a goal: Complete standard compliance (which allows us to cheat)
   *
   * To accomplish this, we follow folly's three-word design, with an overlayed union spanning
   * all three words. We use the right-most byte of the string to categorize it into one of
   * three possible states (assuming a 64bit machine):
   *
   * 1. Small (23 characters or less, not including the null-terminator)
   * 2. Large owned (24 or more characters, with unique ownership over the memory)
   * 3. Large shared (24 or more characters, with shared ownership over the memory)
   *
   * Shared ownership just relies on jank's garbage collector. No additional bookkeping, such
   * as reference counting, is done.
   *
   * Within that right-most byte, these three categories are determined by two dedicated bits.
   * If the most-significant-bit (MSB) is set, the string is large and shared. If the next MSB
   * is also set, the string is large and owned. The remaining 6 bits on that byte are used
   * to store the size of the string, in the case of a small string.
   *
   * Rather than just storing the size, we store the remaining capacity, which is the
   * (max_small_size - size). The benefit of this is that, when the small string is as large
   * as possible, i.e. 23 bytes on a 64bit machine, the remaining capacity will be 0, and the
   * two flag bits will be 0, and thus the byte will be 0 and can act as the null-terminator.
   *
   * In the large case, only the two flag bits of the third word are used. Sharing is done by
   * updating the flag bits on both strings to be shared. We share on both copy construction
   * as well as substring operations. Since share substrings, shared strings may not be
   * null-terminated. We'll lazily own the string if c_str() is called on a shared string, but
   * data() is not expected to return a null-terminated string.
   */
  struct native_persistent_string
  {
    using value_type = char;
    using allocator_type = native_allocator<value_type>;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using size_type = allocator_traits::size_type;
    using traits_type = std::char_traits<value_type>;
    using pointer_type = value_type *;
    using const_pointer_type = value_type const *;
    using iterator = pointer_type;
    using const_iterator = const_pointer_type;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

    constexpr native_persistent_string() noexcept
    {
      set_small_size(0);
    }

    constexpr native_persistent_string(native_persistent_string const &s) noexcept
      : store{ s.store }
    {
      /* NOTE: An if saves us a couple of instructions, versus a switch here. */
      if(s.get_category() != category::small)
      {
        const_cast<native_persistent_string &>(s).store.large.set_category(category::large_shared);
        store.large.set_category(category::large_shared);
      }
    }

    constexpr native_persistent_string(native_persistent_string &&s) noexcept
      : store{ std::move(s.store) }
    {
      s.set_small_size(0);
    }

    template <size_type N>
    constexpr native_persistent_string(value_type const (&new_data)[N]) noexcept
    {
      if constexpr(N <= max_small_size)
      {
        init_small(new_data, N);
      }
      else
      {
        init_large_owned(new_data, N);
      }
    }

    [[gnu::nonnull(2)]] constexpr native_persistent_string(const_pointer_type const s) noexcept
      : native_persistent_string{ s, traits_type::length(s) }
    {
    }

    [[gnu::nonnull(2)]] constexpr native_persistent_string(const_pointer_type const s,
                                                           size_type const size) noexcept
    {
      if(size <= max_small_size)
      {
        init_small(s, size);
      }
      else
      {
        init_large_owned(s, size);
      }
    }

    [[gnu::nonnull(2, 4)]] constexpr native_persistent_string(const_pointer_type const lhs,
                                                              size_type const lhs_size,
                                                              const_pointer_type const rhs,
                                                              size_type const rhs_size) noexcept
    {
      auto const combined_size(lhs_size + rhs_size);
      if(combined_size <= max_small_size)
      {
        init_small(lhs, lhs_size, rhs, rhs_size);
      }
      else
      {
        init_large_owned(lhs, lhs_size, rhs, rhs_size);
      }
    }

    constexpr native_persistent_string(native_persistent_string_view const &s)
      : native_persistent_string{ s.data(), s.size() }
    {
    }

    constexpr native_persistent_string(native_transient_string const &s)
      : native_persistent_string{ s.data(), s.size() }
    {
    }

    constexpr native_persistent_string(native_persistent_string const &s,
                                       size_type const pos,
                                       size_type count)
    {
      auto const s_length(s.size());
      if(s_length < pos) [[unlikely]]
      {
        throw std::runtime_error{ "position outside of string" };
      }
      else if(count == npos || s_length < pos + count)
      {
        count = s_length - pos;
      }

      if(count <= max_small_size)
      {
        init_small(s.data() + pos, count);
      }
      /* If the size difference between our substring and its original string is too great, it's
       * not worth keeping the original string alive just to share the substring. In that case,
       * we deep copy. This prevents relatively small (yet still categoricall large) substrings
       * from a large file keeping that whole file in memory as long as the substrings live. */
      else if((s_length - count) > max_shared_difference)
      {
        init_large_owned(s.store.large.data + pos, count);
      }
      else
      {
        /* NOTE: Not necessarily null-terminated! */
        const_cast<native_persistent_string &>(s).store.large.set_category(category::large_shared);
        init_large_shared(s.store.large.data + pos, count);
      }
    }

    constexpr ~native_persistent_string() noexcept
    {
      destroy();
    }

    /*** Data accessors. ***/
    [[gnu::const]]
    constexpr native_bool empty() const noexcept
    {
      return size() == 0;
    }

    [[gnu::const]]
    constexpr size_type size() const noexcept
    {
      return (get_category() == category::small) ? get_small_size() : store.large.size;
    }

    /* XXX: The contents returned, for large shared strings, may not be null-terminated. If
     * you require that, use c_str(). Whenever possible, use data() and size(). */
    [[gnu::returns_nonnull, gnu::const]]
    constexpr const_pointer_type data() const noexcept
    {
      return (get_category() == category::small) ? store.small : store.large.data;
    }

    /* Always returns a null-terminated string. For shared large strings, we'll allocate
     * and copy the contents upon calling c_str(). If you can use data() and size(), do that. */
    [[gnu::returns_nonnull]]
    constexpr const_pointer_type c_str() const noexcept
    {
      switch(get_category())
      {
        case category::small:
          return store.small;
        case category::large_owned:
          return store.large.data;
        /* For shared strings, we lazily convert to an owned string, since a shared string
         * is not guaranteed to be null-terminated. */
        case category::large_shared:
          {
            /* Shared strings are always large. */
            const_cast<native_persistent_string *>(this)->init_large_owned(store.large.data,
                                                                           store.large.size);
            return store.large.data;
          }
      }
    }

    /*** Searches. ***/
    [[gnu::const]]
    constexpr size_type
    find(native_persistent_string const &pattern, size_type const pos = 0) const noexcept
    {
      return find(pattern.data(), pos, pattern.size());
    }

    [[gnu::const, gnu::nonnull(2)]]
    constexpr size_type
    find(const_pointer_type const pattern, size_type const pos = 0) const noexcept
    {
      return find(pattern, pos, traits_type::length(pattern));
    }

    [[gnu::const, gnu::nonnull(2)]]
    constexpr size_type find(const_pointer_type const pattern,
                             size_type const pos,
                             size_type const pattern_length) const noexcept
    {
      auto const corpus_length(size());
      if(pattern_length == 0) [[unlikely]]
      {
        return pos <= corpus_length ? pos : npos;
      }
      else if(corpus_length <= pos) [[unlikely]]
      {
        return npos;
      }

      auto const pattern_start(pattern[0]);
      auto const corpus_start(data());
      auto const corpus_last(corpus_start + corpus_length);
      auto corpus_pos(corpus_start + pos);
      auto remaining_len(corpus_length - pos);

      while(remaining_len >= pattern_length)
      {
        corpus_pos
          = traits_type::find(corpus_pos, remaining_len - pattern_length + 1, pattern_start);
        if(!corpus_pos)
        {
          return npos;
        }

        /* We compare the full string here, including the first character which we've
         * already matched, since the pattern is likely aligned and comparing from the start
         * will be faster for memcmp. */
        if(traits_type::compare(corpus_pos, pattern, pattern_length) == 0)
        {
          return corpus_pos - corpus_start;
        }
        remaining_len = corpus_last - ++corpus_pos;
      }
      return npos;
    }

    [[gnu::const]]
    constexpr size_type find(value_type c, size_type pos = 0) const noexcept
    {
      size_type ret{ npos };
      auto const length(size());
      if(pos < length) [[likely]]
      {
        auto d(data());
        auto const n(length - pos);
        auto const p(traits_type::find(d + pos, n, c));
        /* TODO: cmov instead of branch here? */
        if(p)
        {
          ret = p - d;
        }
      }
      return ret;
    }

    [[gnu::const]]
    constexpr size_type
    rfind(native_persistent_string const &s, size_type const pos = npos) const noexcept
    {
      return rfind(s.data(), pos, s.size());
    }

    [[gnu::const, gnu::nonnull(2)]]
    constexpr size_type rfind(const_pointer_type const s, size_type const pos = npos) const noexcept
    {
      return rfind(s, pos, traits_type::length(s));
    }

    [[gnu::const, gnu::nonnull(2)]]
    constexpr size_type
    rfind(const_pointer_type const s, size_type pos, size_type const n) const noexcept
    {
      auto const length(size());
      if(n > length)
      {
        return npos;
      }

      pos = std::min(pos, length - n);
      if(n == 0) [[unlikely]]
      {
        return pos;
      }

      auto const beg(data());
      auto it(beg + pos);
      for(;; --it)
      {
        if(*it == *s && traits_type::compare(it, s, n) == 0)
        {
          return it - beg;
        }
        else if(it == beg)
        {
          break;
        }
      }
      return npos;
    }

    [[gnu::const]]
    constexpr size_t rfind(value_type const c, size_type const pos = npos) const noexcept
    {
      auto length(size());
      if(length)
      {
        auto d(data());
        if(--length > pos)
        {
          length = pos;
        }
        for(++length; length-- > 0;)
        {
          if(d[length] == c)
          {
            return length;
          }
        }
      }
      return npos;
    }

    /*** Immutable modifications. ***/
    constexpr native_persistent_string
    substr(size_type const pos = 0, size_type const count = npos) const
    {
      return { *this, pos, count };
    }

    /*** Mutations. ***/
    constexpr native_persistent_string &operator=(native_persistent_string const &rhs)
    {
      if(this == &rhs)
      {
        return *this;
      }

      destroy();

      store = rhs.store;
      if(rhs.get_category() == category::large_owned)
      {
        const_cast<native_persistent_string &>(rhs).store.large.set_category(
          category::large_shared);
        store.large.set_category(category::large_shared);
      }

      return *this;
    }

    constexpr native_persistent_string &operator=(const_pointer_type const rhs)
    {
      destroy();

      auto const length(traits_type::length(rhs));
      if(length <= max_small_size)
      {
        init_small(rhs, length);
      }
      else
      {
        init_large_owned(rhs, length);
      }

      return *this;
    }

    constexpr native_persistent_string &operator=(native_transient_string const &rhs)
    {
      destroy();

      auto const length(rhs.size());
      if(length <= max_small_size)
      {
        init_small(rhs.data(), length);
      }
      else
      {
        init_large_owned(rhs.data(), length);
      }

      return *this;
    }

    /*** Comparisons. ***/
    [[gnu::const]]
    constexpr native_bool
    operator!=(native_persistent_string const &s) const noexcept
    {
      auto const length(size());
      return length != s.size() || traits_type::compare(data(), s.data(), length);
    }

    [[gnu::const]]
    constexpr native_bool
    operator==(native_persistent_string const &s) const noexcept
    {
      return !(*this != s);
    }

    [[gnu::const, gnu::nonnull(2)]]
    constexpr native_bool
    operator!=(const_pointer_type const s) const noexcept
    {
      auto const length(traits_type::length(s));
      return size() != length || traits_type::compare(data(), s, length);
    }

    [[gnu::const, gnu::nonnull(2)]]
    constexpr native_bool
    operator==(const_pointer_type const s) const noexcept
    {
      return !(*this != s);
    }

    [[gnu::const]]
    constexpr int compare(native_persistent_string const &s) const
    {
      auto const length(size());
      auto const s_length(s.size());
      auto const common_length(std::min(length, s_length));

      int r(traits_type::compare(data(), s.data(), common_length));
      r = !r ? static_cast<int>(length - s_length) : r;
      return r;
    }

    /*** Iterators. ***/
    [[gnu::const]]
    constexpr const_iterator begin() const noexcept
    {
      return data();
    }

    [[gnu::const]]
    constexpr const_iterator cbegin() const noexcept
    {
      return begin();
    }

    [[gnu::const]]
    constexpr const_iterator end() const noexcept
    {
      return data() + size();
    }

    [[gnu::const]]
    constexpr const_iterator cend() const
    {
      return end();
    }

    [[gnu::const]]
    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    [[gnu::const]]
    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return rbegin();
    }

    [[gnu::const]]
    constexpr const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    [[gnu::const]]
    constexpr const_reverse_iterator crend() const noexcept
    {
      return rend();
    }

    /*** Conversions. ***/
    constexpr operator native_persistent_string_view() const
    {
      return { data(), size() };
    }

    /*** Hashing. ***/
    constexpr native_integer to_hash() const noexcept
    {
      if(store.hash != 0)
      {
        return store.hash;
      }

      /* https://github.com/openjdk/jdk/blob/7e30130e354ebfed14617effd2a517ab2f4140a5/src/java.base/share/classes/java/lang/StringLatin1.java#L194 */
      auto const ptr(data());
      for(size_t i{}; i != size(); ++i)
      {
        store.hash = 31 * store.hash + (ptr[i] & 0xff);
      }
      return store.hash;
    }

  private:
    static constexpr native_bool is_little_endian{ std::endian::native == std::endian::little };

    enum class category : uint8_t
    {
      small = 0,
      large_shared = is_little_endian ? 0b10000000 : 0b00000001,
      large_owned = is_little_endian ? 0b11000000 : 0b00000011
    };

    struct large_storage
    {
      pointer_type data;
      size_type size;
      size_type extra;

      constexpr void set_category(category const new_category) noexcept
      {
        extra = static_cast<size_type>(new_category) << category_shift;
      }
    };

    static constexpr uint8_t last_char_index{ sizeof(large_storage) - 1 };
    static constexpr uint8_t max_small_size{ last_char_index / sizeof(value_type) };
    static constexpr uint16_t max_shared_difference{ 512 };
    /* The size is shifted to/from storage, to account for the 2 extra data bits. */
    static constexpr uint8_t small_shift{ is_little_endian ? 0 : 2 };
    static constexpr uint8_t category_extraction_mask{ is_little_endian ? 0b11000000 : 0b00000011 };
    static constexpr uint8_t category_shift{ (sizeof(size_type) - 1) * 8 };

    /* Our storage provides three ways of accessing the same data:
     *   1. Direct bytes (used to access the right-most flag byte)
     *   2. In-place char buffer (used for small categories)
     *   3. As a large_storage instance, containing a pointer, size, and capacity
     */
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init) */
    struct storage : allocator_type
    {
      /* TODO: What if we store a max of 22 chars and dedicate a byte for flags with no masking? */
      union {
        uint8_t bytes[sizeof(large_storage)];
        value_type small[sizeof(large_storage) / sizeof(value_type)];
        large_storage large;
      };

      /* TODO: Benchmark benefit of storing this hash vs calculating it each time. */
      mutable native_integer hash{};
    };

    constexpr void destroy()
    {
      /* NOTE: No performance difference between if/switch here. */
      if(get_category() == category::large_owned)
      {
        allocator_traits::deallocate(store, store.large.data, store.large.size + 1);
      }
    }

    [[gnu::const]]
    constexpr category get_category() const noexcept
    {
      return static_cast<category>(store.bytes[last_char_index] & category_extraction_mask);
    }

    [[gnu::const]]
    constexpr size_type get_small_size() const noexcept
    {
      assert(get_category() == category::small);
      auto const small_shifted(static_cast<size_type>(store.small[max_small_size]) >> small_shift);
      assert(max_small_size >= small_shifted);
      return max_small_size - small_shifted;
    }

    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    constexpr void set_small_size(size_type const s) noexcept
    {
      assert(s <= max_small_size);
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index) */
      store.small[s] = 0;
      store.small[max_small_size] = value_type((max_small_size - s) << small_shift);
      assert(get_category() == category::small && size() == s);
    }

    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    constexpr void init_small(const_pointer_type const data, uint8_t const size) noexcept
    {
      /* If `data` is word-aligned, we can do three quick word copies. */
      if((std::bit_cast<size_type>(data) & (sizeof(size_type) - 1)) == 0)
      {
        auto const aligned_data(std::assume_aligned<sizeof(size_type)>(data));
        uint8_t const byte_size(size * sizeof(value_type));
        constexpr uint8_t word_width{ sizeof(size_type) };
        /* NOTE: We're writing in reverse order here, but it uses one less instruction and
         * is marginally faster than duplicating the code each each case to write in order. */
        switch((byte_size + word_width - 1) / word_width)
        {
          case 3:
            store.large.extra = std::bit_cast<size_type const *>(aligned_data)[2];
          case 2:
            store.large.size = std::bit_cast<size_type const *>(aligned_data)[1];
          case 1:
            store.large.data = std::bit_cast<pointer_type *>(aligned_data)[0];
          case 0:
            break;
        }
      }
      else
      {
        traits_type::copy(store.small, data, size);
      }

      set_small_size(size);
    }

    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    constexpr void init_small(const_pointer_type const lhs,
                              uint8_t const lhs_size,
                              const_pointer_type const rhs,
                              uint8_t const rhs_size) noexcept
    {
      assert(lhs_size + rhs_size <= max_small_size);
      traits_type::copy(store.small, lhs, lhs_size);
      traits_type::copy(store.small + lhs_size, rhs, rhs_size);
      set_small_size(lhs_size + rhs_size);
    }

    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    constexpr void init_large_shared(const_pointer_type const data, size_type const size) noexcept
    {
      /* NOTE: This is likely NOT null-terminated. We need to look out for this in c_str(). */
      assert(max_small_size < size);
      store.large.data = const_cast<pointer_type>(data);
      store.large.size = size;
      store.large.set_category(category::large_shared);
    }

    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    constexpr void init_large_owned(const_pointer_type const data, size_type const size) noexcept
    {
      assert(max_small_size < size);
      /* TODO: Apply gnu::malloc to this fn. */
      store.large.data = std::assume_aligned<sizeof(pointer_type)>(store.allocate(size + 1));
      traits_type::copy(store.large.data, data, size);
      store.large.data[size] = 0;
      store.large.size = size;
      store.large.set_category(category::large_owned);
    }

    [[gnu::always_inline, gnu::flatten, gnu::hot]]
    constexpr void init_large_owned(const_pointer_type const lhs,
                                    size_type const lhs_size,
                                    const_pointer_type const rhs,
                                    size_type const rhs_size) noexcept
    {
      auto const size(lhs_size + rhs_size);
      assert(max_small_size < size);
      store.large.data = std::assume_aligned<sizeof(pointer_type)>(store.allocate(size + 1));
      traits_type::copy(store.large.data, lhs, lhs_size);
      traits_type::copy(store.large.data + lhs_size, rhs, rhs_size);
      store.large.data[size] = 0;
      store.large.size = size;
      store.large.set_category(category::large_owned);
    }

    storage store;
  };

  [[gnu::const]]
  constexpr native_bool
  operator<(native_persistent_string const &lhs, native_persistent_string const &rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }

  constexpr native_persistent_string
  operator+(native_persistent_string const &lhs, native_persistent_string const &rhs) noexcept
  {
    return { lhs.data(), lhs.size(), rhs.data(), rhs.size() };
  }

  constexpr native_persistent_string
  operator+(native_persistent_string::const_pointer_type const lhs,
            native_persistent_string const &rhs) noexcept
  {
    return { lhs, native_persistent_string::traits_type::length(lhs), rhs.data(), rhs.size() };
  }

  constexpr native_persistent_string
  operator+(native_persistent_string const &lhs,
            native_persistent_string::const_pointer_type const rhs) noexcept
  {
    return { lhs.data(), lhs.size(), rhs, native_persistent_string::traits_type::length(rhs) };
  }

  constexpr native_persistent_string
  operator+(native_persistent_string const &lhs,
            native_persistent_string::value_type const rhs) noexcept
  {
    return { lhs.data(), lhs.size(), &rhs, 1 };
  }

  constexpr std::ostream &operator<<(std::ostream &os, native_persistent_string const &s)
  {
    return os << static_cast<native_persistent_string_view>(s);
  }
}

template <>
struct fmt::formatter<jank::native_persistent_string> : private formatter<fmt::string_view>
{
  using formatter<fmt::string_view>::parse;

  template <typename Context>
  auto format(jank::native_persistent_string const &s, Context &ctx) const
  {
    return formatter<fmt::string_view>::format({ s.data(), s.size() }, ctx);
  }
};

namespace std
{
  template <>
  struct hash<jank::native_persistent_string>
  {
    size_t operator()(jank::native_persistent_string const &s) const
    {
      return s.to_hash();
    }
  };

  template <>
  struct formatter<jank::native_persistent_string> : formatter<std::string_view>
  {
    template <typename Context>
    auto format(jank::native_persistent_string const &s, Context &ctx) const
    {
      return formatter<std::string_view>::format({ s.data(), s.size() }, ctx);
    }
  };
}
