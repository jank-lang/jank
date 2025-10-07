#pragma once

#include <limits>
#include <string>

#include <jtl/primitive.hpp>
#include <jtl/assert.hpp>

namespace jtl
{
  struct string_builder;

  struct immutable_string_view
  {
    using value_type = char;
    using size_type = usize;
    using traits_type = std::char_traits<value_type>;
    using pointer_type = value_type *;
    using const_pointer_type = value_type const *;
    using iterator = pointer_type;
    using const_iterator = const_pointer_type;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos{ std::numeric_limits<size_type>::max() };

    constexpr immutable_string_view() noexcept = delete;
    constexpr immutable_string_view(immutable_string_view const &) noexcept = default;
    constexpr immutable_string_view(immutable_string_view &&) noexcept = default;

    [[gnu::nonnull(2)]]
    constexpr immutable_string_view(const_pointer_type const s) noexcept
      : immutable_string_view{ s, traits_type::length(s) }
    {
    }

    [[gnu::nonnull(2)]]
    constexpr immutable_string_view(const_pointer_type const s, size_type const size) noexcept
      : ptr{ s }
      , len{ size }
    {
      jank_debug_assert(ptr);
    }

    [[gnu::nonnull(2)]]
    constexpr immutable_string_view(const_pointer_type const begin,
                                    const_pointer_type const end) noexcept
      : ptr{ begin }
      , len{ static_cast<size_type>(end - begin) }
    {
      jank_debug_assert(ptr);
    }

    constexpr immutable_string_view(std::string const &s) noexcept
      : ptr{ s.data() }
      , len{ s.size() }
    {
      jank_debug_assert(ptr);
    }

    constexpr immutable_string_view(std::string_view const &s) noexcept
      : ptr{ s.data() }
      , len{ s.size() }
    {
      jank_debug_assert(ptr);
    }

    constexpr bool empty() const noexcept
    {
      return size() == 0;
    }

    constexpr size_type size() const noexcept
    {
      return len;
    }

    /* XXX: The contents returned may not be null-terminated. If you require
     * that, build a jtl::immutable_string and use c_str() on it. */
    [[gnu::returns_nonnull]]
    constexpr const_pointer_type data() const noexcept
    {
      return ptr;
    }

    constexpr value_type operator[](size_type const index) const noexcept
    {
      jank_debug_assert(index < len);
      return ptr[index];
    }

    /*** Comparisons. ***/
    constexpr bool operator!=(immutable_string_view const &s) const noexcept
    {
      auto const length(size());
      return length != s.size() || traits_type::compare(data(), s.data(), length);
    }

    constexpr bool operator==(immutable_string_view const &s) const noexcept
    {
      return !(*this != s);
    }

    [[gnu::nonnull(2)]]
    constexpr bool operator!=(const_pointer_type const s) const noexcept
    {
      auto const length(traits_type::length(s));
      return size() != length || traits_type::compare(data(), s, length);
    }

    [[gnu::nonnull(2)]]
    constexpr bool operator==(const_pointer_type const s) const noexcept
    {
      return !(*this != s);
    }

    constexpr int compare(immutable_string_view const &s) const
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
    {
      return data();
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return begin();
    }

    constexpr const_iterator end() const noexcept
    {
      return data() + size();
    }

    constexpr const_iterator cend() const
    {
      return end();
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return rbegin();
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
      return rend();
    }

    /*** Searches. ***/
    constexpr size_type
    find(immutable_string_view const &pattern, size_type const pos = 0) const noexcept
    {
      return find(pattern.data(), pos, pattern.size());
    }

    [[gnu::nonnull(2)]]
    constexpr size_type
    find(const_pointer_type const pattern, size_type const pos = 0) const noexcept
    {
      return find(pattern, pos, traits_type::length(pattern));
    }

    [[gnu::nonnull(2)]]
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

    constexpr size_type
    rfind(immutable_string_view const &s, size_type const pos = npos) const noexcept
    {
      return rfind(s.data(), pos, s.size());
    }

    [[gnu::nonnull(2)]]
    constexpr size_type rfind(const_pointer_type const s, size_type const pos = npos) const noexcept
    {
      return rfind(s, pos, traits_type::length(s));
    }

    [[gnu::nonnull(2)]]
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

    constexpr size_type rfind(value_type const c, size_type const pos = npos) const noexcept
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

    constexpr bool starts_with(value_type const c) const noexcept
    {
      return len > 0 && ptr[0] == c;
    }

    constexpr bool starts_with(const_pointer_type const s) const noexcept
    {
      auto const this_sz(size());
      auto const s_sz(traits_type::length(s));
      if(this_sz < s_sz)
      {
        return false;
      }
      return traits_type::compare(data(), s, s_sz) == 0;
    }

    constexpr bool starts_with(jtl::immutable_string_view const &s) const noexcept
    {
      auto const this_sz(size());
      auto const s_sz(s.size());
      if(this_sz < s_sz)
      {
        return false;
      }
      /* NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) */
      return traits_type::compare(data(), s.data(), s_sz) == 0;
    }

    constexpr bool ends_with(value_type const c) const noexcept
    {
      auto const s(size());
      return s > 0 && data()[s - 1] == c;
    }

    constexpr bool ends_with(const_pointer_type const s) const noexcept
    {
      auto const this_sz(size());
      auto const s_sz(traits_type::length(s));
      if(this_sz < s_sz)
      {
        return false;
      }
      return traits_type::compare(data() + this_sz - s_sz, s, s_sz) == 0;
    }

    constexpr bool ends_with(jtl::immutable_string_view const &s) const noexcept
    {
      auto const this_sz(size());
      auto const s_sz(s.size());
      if(this_sz < s_sz)
      {
        return false;
      }
      /* NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) */
      return traits_type::compare(data() + this_sz - s_sz, s.data(), s_sz) == 0;
    }

    constexpr bool contains(value_type const c) const noexcept
    {
      /* NOLINTNEXTLINE(readability-container-contains) */
      return find(c) != npos;
    }

    constexpr bool contains(const_pointer_type const s) const noexcept
    {
      /* NOLINTNEXTLINE(readability-container-contains) */
      return find(s) != npos;
    }

    /*** Immutable modifications. ***/
    constexpr immutable_string_view
    substr(size_type const pos = 0, size_type const count = npos) const
    {
      jank_debug_assert(pos <= len);
      if(count == npos)
      {
        return { ptr + pos, len - pos };
      }

      jank_debug_assert(count <= len);
      return { ptr + pos, count };
    }

    constexpr operator std::string_view() const
    {
      return { ptr, len };
    }

    /*** Mutations. ***/
    constexpr immutable_string_view &operator=(immutable_string_view const &rhs) = default;
    constexpr immutable_string_view &operator=(immutable_string_view &&rhs) = default;

    value_type const *ptr{};
    size_type len{};
  };

  //constexpr std::ostream &operator<<(std::ostream &os, immutable_string_view const &s)
  //{
  //  for(auto const c : s)
  //  {
  //    os << c;
  //  }
  //  return os;
  //}
}

namespace jank::hash
{
  u32 string(jtl::immutable_string_view const &input);
}

namespace std
{
  template <>
  struct hash<jtl::immutable_string_view>
  {
    size_t operator()(jtl::immutable_string_view const &s) const
    {
      return jank::hash::string(s);
    }
  };
}
