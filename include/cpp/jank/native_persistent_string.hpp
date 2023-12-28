#pragma once

#include <bit>
#include <fmt/format.h>

namespace jank
{
  struct native_persistent_string
  {
    using value_type = char;
    using allocator_type = native_allocator<value_type>;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using size_type = allocator_traits::size_type;
    using traits_type = std::char_traits<value_type>;
    using pointer_type = value_type*;
    using const_pointer_type = value_type const*;
    using iterator = pointer_type;
    using const_iterator = const_pointer_type;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

    constexpr native_persistent_string() noexcept
    { set_small_size(0); }

    constexpr native_persistent_string(native_persistent_string const &s) noexcept
      : store{ s.store }
    {
      /* NOTE: An if saves us a couple of instructions, versus a switch here. */
      if(s.get_category() != category::small)
      {
        const_cast<native_persistent_string&>(s).store.large.set_category(category::large_shared);
        store.large.set_category(category::large_shared);
      }
    }

    constexpr native_persistent_string(native_persistent_string &&s) noexcept
      : store{ std::move(s.store) }
    { s.set_small_size(0); }

    template <size_type N>
    constexpr native_persistent_string(value_type const (&new_data)[N]) noexcept
    {
      if constexpr(N <= max_small_size)
      { init_small(new_data, N); }
      else
      { init_large_owned(new_data, N); }
    }

    constexpr native_persistent_string(const_pointer_type const s) noexcept
      : native_persistent_string{ s, traits_type::length(s) }
    { }

    constexpr native_persistent_string(const_pointer_type const s, size_type const size) noexcept
    {
      if(size <= max_small_size)
      { init_small(s, size); }
      else
      { init_large_owned(s, size); }
    }

    constexpr native_persistent_string
    (
      const_pointer_type const lhs, size_type const lhs_size,
      const_pointer_type const rhs, size_type const rhs_size
    ) noexcept
    {
      auto const combined_size(lhs_size + rhs_size);
      if(combined_size <= max_small_size)
      { init_small(lhs, lhs_size, rhs, rhs_size); }
      else
      { init_large_owned(lhs, lhs_size, rhs, rhs_size); }
    }

    constexpr native_persistent_string(native_persistent_string_view const &s)
      : native_persistent_string{ s.data(), s.size() }
    { }

    constexpr native_persistent_string(native_transient_string const &s)
      : native_persistent_string{ s.data(), s.size() }
    { }

    constexpr native_persistent_string(native_persistent_string const &s, size_type const pos, size_type count)
    {
      auto const s_length(s.size());
      if(s_length < pos) [[unlikely]]
      { throw std::runtime_error{ "position outside of string" }; }
      else if(count == npos || s_length < pos + count)
      { count = s_length - pos; }

      if(count <= max_small_size)
      { init_small(s.data() + pos, count); }
      else
      {
        /* NOTE: Not necessarily null-terminated! */
        const_cast<native_persistent_string&>(s).store.large.set_category(category::large_shared);
        init_large_shared(s.data() + pos, count);
      }
    }

    constexpr ~native_persistent_string() noexcept
    { destroy(); }

    /*** Data accessors. ***/
    constexpr native_bool empty() const noexcept
    { return size() == 0; }

    constexpr size_type size() const noexcept
    { return (get_category() == category::small) ? get_small_size() : store.large.size; }

    constexpr size_type capacity() const noexcept
    {
      if(get_category() == category::small)
      { return max_small_size; }
      else
      { return store.large.size; }
    }

    constexpr const_pointer_type data() const noexcept
    { return (get_category() == category::small) ? store.small : store.large.data; }
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
          const_cast<native_persistent_string*>(this)->init_large_owned(store.large.data, size());
          return store.large.data;
        }
      }
    }

    /*** Searches. ***/
    constexpr size_type find(native_persistent_string const &pattern, size_type const pos = 0) const noexcept
    { return find(pattern.data(), pos, pattern.size()); }
    constexpr size_type find(const_pointer_type const pattern, size_type const pos = 0) const noexcept
    { return find(pattern, pos, traits_type::length(pattern)); }
    constexpr size_type find
    (
      const_pointer_type const pattern,
      size_type const pos,
      size_type const pattern_length
    ) const noexcept
    {
      auto const corpus_length(size());
      if(pattern_length == 0) [[unlikely]]
      { return pos <= corpus_length ? pos : npos; }
      else if(corpus_length <= pos) [[unlikely]]
      { return npos; }

      auto const pattern_start(pattern[0]);
      auto const corpus_start(data());
      auto const corpus_last(corpus_start + corpus_length);
      auto corpus_pos(corpus_start + pos);
      auto remaining_len(corpus_length - pos);

      while(remaining_len >= pattern_length)
      {
        corpus_pos = traits_type::find(corpus_pos, remaining_len - pattern_length + 1, pattern_start);
        if(!corpus_pos)
        { return npos; }

        /* We compare the full string here, including the first character which we've
         * already matched, since the pattern is likely aligned and comparing from the start
         * will be faster for memcmp. */
        if(traits_type::compare(corpus_pos, pattern, pattern_length) == 0)
        { return corpus_pos - corpus_start; }
        remaining_len = corpus_last - ++corpus_pos;
      }
      return npos;
    }

    constexpr size_type find(value_type c, size_type pos = 0) const noexcept
    {
      size_type ret{ npos };
      auto const length(size());
      if(pos < length) [[likely]]
      {
        auto d(data());
        auto const n(length - pos);
        auto const p(traits_type::find(d + pos, n, c));
        if(p)
        { ret = p - d; }
      }
      return ret;
    }

    constexpr size_type rfind(native_persistent_string const &s, size_type const pos = npos) const noexcept
    { return rfind(s.data(), pos, s.size()); }
    constexpr size_type rfind(const_pointer_type const s, size_type const pos = npos) const noexcept
    { return rfind(s, pos, traits_type::length(s)); }
    constexpr size_type rfind(const_pointer_type const s, size_type pos, size_type const n) const noexcept
    {
      auto const length(size());
      if(n > length)
      { return npos; }

      pos = std::min(pos, length - n);
      if(n == 0) [[unlikely]]
      { return pos; }

      auto const beg(data());
      auto it(beg + pos);
      for(;; --it)
      {
        if(*it == *s && traits_type::compare(it, s, n) == 0)
        { return it - beg; }
        else if(it == beg)
        { break; }
      }
      return npos;
    }

    constexpr size_t rfind(value_type const c, size_type const pos = npos) const noexcept
    {
      auto length(size());
      if(length)
      {
        auto d(data());
        if(--length > pos)
        { length = pos; }
        for(++length; length-- > 0; )
        {
          if(d[length] == c)
          { return length; }
        }
      }
      return npos;
    }

    /*** Immutable modifications. ***/
    constexpr native_persistent_string substr(size_type const pos = 0, size_type const count = npos) const
    { return { *this, pos, count }; }

    /*** Mutations. ***/
    constexpr native_persistent_string& operator =(native_persistent_string const &rhs)
    {
      if(this == &rhs)
      { return *this; }

      destroy();

      store = rhs.store;
      if(rhs.get_category() == category::large_owned)
      {
        const_cast<native_persistent_string&>(rhs).store.large.set_category(category::large_shared);
        store.large.set_category(category::large_shared);
      }

      return *this;
    }

    constexpr native_persistent_string& operator =(const_pointer_type const rhs)
    {
      destroy();

      auto const length(traits_type::length(rhs));
      if(length <= max_small_size)
      { init_small(rhs, length); }
      else
      { init_large_owned(rhs, length); }

      return *this;
    }

    constexpr native_persistent_string& operator =(native_transient_string const &rhs)
    {
      destroy();

      auto const length(rhs.size());
      if(length <= max_small_size)
      { init_small(rhs.data(), length); }
      else
      { init_large_owned(rhs.data(), length); }

      return *this;
    }

    /*** Comparisons. ***/
    constexpr native_bool operator !=(native_persistent_string const &s) const noexcept
    {
      auto const length(size());
      return length != s.size() || traits_type::compare(data(), s.data(), length);
    }
    constexpr native_bool operator ==(native_persistent_string const &s) const noexcept
    { return !(*this != s); }

    constexpr native_bool operator !=(const_pointer_type const s) const noexcept
    {
      auto const length(traits_type::length(s));
      return size() != length || traits_type::compare(data(), s, length);
    }
    constexpr native_bool operator ==(const_pointer_type const s) const noexcept
    { return !(*this != s); }

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
    constexpr const_iterator begin() const noexcept
    { return data(); }

    constexpr const_iterator cbegin() const noexcept
    { return begin(); }

    constexpr const_iterator end() const noexcept
    { return data() + size(); }

    constexpr const_iterator cend() const
    { return end(); }

    constexpr const_reverse_iterator rbegin() const noexcept
    { return const_reverse_iterator(end()); }

    constexpr const_reverse_iterator crbegin() const noexcept
    { return rbegin(); }

    constexpr const_reverse_iterator rend() const noexcept
    { return const_reverse_iterator(begin()); }

    constexpr const_reverse_iterator crend() const noexcept
    { return rend(); }

    /*** Conversions. ***/
    constexpr operator native_persistent_string_view() const
    { return { data(), size() }; }

    /*** Hashing. ***/
    constexpr native_integer to_hash() const noexcept
    {
      if(hash != 0)
      { return hash; }

      /* https://github.com/openjdk/jdk/blob/7e30130e354ebfed14617effd2a517ab2f4140a5/src/java.base/share/classes/java/lang/StringLatin1.java#L194 */
      auto const ptr(data());
      for(size_t i{}; i != size(); ++i)
      { hash = 31 * hash + (ptr[i] & 0xff); }
      return hash;
    }

  private:
    static constexpr native_bool is_little_endian{ std::endian::native == std::endian::little };

    enum class category : uint8_t
    {
      small = 0,
      large_shared = is_little_endian ? 0b10000000 : 0b00000001,
      large_owned = is_little_endian ? 0b11000000 : 0b00000011
    };

    struct Large
    {
      pointer_type data;
      size_type size;
      size_type capacity;

      constexpr size_type get_capacity() const noexcept
      {
        if constexpr(is_little_endian)
        { return capacity & capacity_extraction_mask; }
        else
        { return capacity >> 2; }
      }

      constexpr void set_capacity(size_type const new_capacity, category const new_category) noexcept
      {
        if constexpr(is_little_endian)
        { capacity = new_capacity | (static_cast<size_type>(new_category) << category_shift); }
        else
        { capacity = (new_capacity << 2) | static_cast<size_type>(new_category); }
      }

      constexpr void set_category(category const new_category) noexcept
      {
        if constexpr(is_little_endian)
        { capacity = (static_cast<size_type>(new_category) << category_shift); }
        else
        { capacity = (capacity << 2) | static_cast<size_type>(new_category); }
      }
    };

    static constexpr size_type last_char_index{ sizeof(Large) - 1 };
    static constexpr size_type max_small_size{ last_char_index / sizeof(value_type) };
    /* The size is shifted to/from storage, to account for the 2 extra data bits. */
    static constexpr size_type small_shift{ is_little_endian ? 0 : 2 };
    static constexpr uint8_t category_extraction_mask{ is_little_endian ? 0b11000000 : 0b00000011 };
    static constexpr size_type category_shift{ (sizeof(size_type) - 1) * 8 };
    static constexpr size_type capacity_extraction_mask
    {
      is_little_endian
        ? ~(size_type(category_extraction_mask) << category_shift)
        : 0 /* unused */
    };

    /* Our storage provides three ways of accessing the same data:
     *   1. Direct bytes (used to access the right-most flag byte)
     *   2. In-place char buffer (used for small categories)
     *   3. As a Large instance, containing a pointer, size, and capacity
     */
    struct storage : allocator_type
    {
      union
      {
        uint8_t bytes[sizeof(Large)];
        value_type small[sizeof(Large) / sizeof(value_type)];
        Large large;
      };
    };

    constexpr void destroy()
    {
      /* NOTE: No performance difference between if/switch here. */
      if(get_category() == category::large_owned)
      { allocator_traits::deallocate(store, store.large.data, size() + 1); }
    }

    constexpr category get_category() const noexcept
    { return static_cast<category>(store.bytes[last_char_index] & category_extraction_mask); }

    constexpr size_type get_small_size() const noexcept
    {
      assert(get_category() == category::small);
      auto const small_shifted(static_cast<size_type>(store.small[max_small_size]) >> small_shift);
      assert(max_small_size >= small_shifted);
      return max_small_size - small_shifted;
    }

    constexpr void set_small_size(size_type const s) noexcept
    {
      assert(s <= max_small_size);
      store.small[s] = 0;
      store.small[max_small_size] = value_type((max_small_size - s) << small_shift);
      assert(get_category() == category::small && size() == s);
    }

    constexpr void init_small(const_pointer_type const data, size_type const size) noexcept
    {
      /* If `data` is word-aligned, we can do three quick word copies. */
      if((std::bit_cast<size_type>(data) & (sizeof(size_type) - 1)) == 0)
      {
        auto const aligned_data(std::assume_aligned<sizeof(size_type)>(data));
        size_type const byte_size{ size * sizeof(value_type) };
        constexpr size_type word_width{ sizeof(size_type) };
        /* NOTE: We're writing in reverse order here, but it uses one less instruction and
         * is marginally faster than duplicating the code each each case to write in order. */
        switch((byte_size + word_width - 1) / word_width)
        {
          case 3:
            store.large.capacity = std::bit_cast<size_type const*>(aligned_data)[2];
          case 2:
            store.large.size = std::bit_cast<size_type const*>(aligned_data)[1];
          case 1:
            store.large.data = std::bit_cast<pointer_type*>(aligned_data)[0];
          case 0:
            break;
        }
      }
      else
      { traits_type::copy(store.small, data, size); }

      set_small_size(size);
    }

    constexpr void init_small
    (
      const_pointer_type const lhs, size_type const lhs_size,
      const_pointer_type const rhs, size_type const rhs_size
    ) noexcept
    {
      assert(lhs_size + rhs_size <= max_small_size);
      traits_type::copy(store.small, lhs, lhs_size);
      traits_type::copy(store.small + lhs_size, rhs, rhs_size);
      set_small_size(lhs_size + rhs_size);
    }

    constexpr void init_large_shared(const_pointer_type const data, size_type const size) noexcept
    {
      /* NOTE: This is likely NOT null-terminated. We need to look out for this in c_str(). */
      assert(max_small_size < size);
      store.large.data = const_cast<pointer_type>(data);
      store.large.size = size;
      store.large.set_capacity(size / sizeof(value_type) - 1, category::large_shared);
    }

    constexpr void init_large_owned(const_pointer_type const data, size_type const size) noexcept
    {
      assert(max_small_size < size);
      store.large.data = std::assume_aligned<sizeof(void*)>(store.allocate(size + 1));
      traits_type::copy(store.large.data, data, size);
      store.large.data[size] = 0;
      store.large.size = size;
      store.large.set_capacity(size / sizeof(value_type) - 1, category::large_owned);
    }

    constexpr void init_large_owned
    (
      const_pointer_type const lhs, size_type const lhs_size,
      const_pointer_type const rhs, size_type const rhs_size
    ) noexcept
    {
      auto const size(lhs_size + rhs_size);
      assert(max_small_size < size);
      store.large.data = std::assume_aligned<sizeof(void*)>(store.allocate(size + 1));
      traits_type::copy(store.large.data, lhs, lhs_size);
      traits_type::copy(store.large.data + lhs_size, rhs, rhs_size);
      store.large.data[size] = 0;
      store.large.size = size;
      store.large.set_capacity(size / sizeof(value_type) - 1, category::large_owned);
    }

    storage store;
    mutable native_integer hash{};
  };

  constexpr native_bool operator <(native_persistent_string const &lhs, native_persistent_string const &rhs) noexcept
  { return lhs.compare(rhs) < 0; }

  constexpr native_persistent_string operator +(native_persistent_string const &lhs, native_persistent_string const &rhs) noexcept
  { return { lhs.data(), lhs.size(), rhs.data(), rhs.size() }; }

  constexpr native_persistent_string operator +(native_persistent_string::const_pointer_type const lhs, native_persistent_string const &rhs) noexcept
  { return { lhs, native_persistent_string::traits_type::length(lhs), rhs.data(), rhs.size() }; }

  constexpr native_persistent_string operator +(native_persistent_string const &lhs, native_persistent_string::const_pointer_type const rhs) noexcept
  { return { lhs.data(), lhs.size(), rhs, native_persistent_string::traits_type::length(rhs) }; }

  constexpr native_persistent_string operator +(native_persistent_string const &lhs, native_persistent_string::value_type const rhs) noexcept
  { return { lhs.data(), lhs.size(), &rhs, 1 }; }

  constexpr std::ostream& operator <<(std::ostream &os, native_persistent_string const &s)
  { return os << static_cast<native_persistent_string_view>(s); }
}

template <>
struct fmt::formatter<jank::native_persistent_string> : private formatter<fmt::string_view>
{
  using formatter<fmt::string_view>::parse;

  template <typename Context>
  typename Context::iterator format(jank::native_persistent_string const &s, Context &ctx) const
  { return formatter<fmt::string_view>::format({ s.data(), s.size() }, ctx); }
};

namespace std
{
  template <>
  struct hash<jank::native_persistent_string>
  {
    size_t operator()(jank::native_persistent_string const &s) const
    { return s.to_hash(); }
  };

  template <>
  struct formatter<jank::native_persistent_string> : formatter<std::string_view>
  {
    template<class FormatContext>
    auto format(jank::native_persistent_string const &s, FormatContext &ctx) const
    { return formatter<std::string_view>::format({ s.data(), s.size() }, ctx); }
  };
}
