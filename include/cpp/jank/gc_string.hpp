#pragma once

#include <boost/algorithm/searching/boyer_moore.hpp>

namespace jank
{
  struct gc_string
  {
    using value_type = char;
    using size_type = size_t;
    using traits_type = std::char_traits<value_type>;
    using pointer_type = value_type*;
    using const_pointer_type = value_type const*;
    //using allocator_type = std::allocator<value_type>;
    using allocator_type = native_allocator<value_type>;
    using allocator_traits = std::allocator_traits<allocator_type>;

    static constexpr size_type npos{ std::numeric_limits<size_type>::max() };
    static constexpr size_type local_capacity{ 15 / sizeof(value_type) };

    constexpr gc_string() noexcept
      : ptr{ local_buf }
    {
      use_local_data();
      set_length(0);
    }

    constexpr gc_string(gc_string const &s) noexcept
      : ptr{ s.ptr }
    {
      if(s.is_local())
      {
        ptr.p = local_buf;
        /* Copy from the length member onward, to the end of the object. */
        memcpy(&length, &s.length, sizeof(gc_string) - sizeof(ptr));
      }
      else
      {
        length = s.length;
        allocated_capacity = s.allocated_capacity;
      }
    }

    template <size_type N>
    constexpr gc_string(value_type const (&new_data)[N])
      : ptr{ local_buf }
    {
      const_pointer_type const end{ new_data + N };
      construct(new_data, end, std::forward_iterator_tag{});
    }

    constexpr gc_string(value_type const *new_data)
      : ptr{ local_buf }
    {
      assert(new_data);
      auto const end(new_data + traits_type::length(new_data));
      construct(new_data, end, std::forward_iterator_tag{});
    }

    constexpr gc_string(value_type const *new_data, size_type const length)
      : ptr{ local_buf }
    {
      assert(new_data);
      auto const end(new_data + length);
      construct(new_data, end, std::forward_iterator_tag{});
    }

    constexpr gc_string(gc_string const &s, size_type const pos, size_type count)
    {
      if(s.length < pos) [[unlikely]]
      { throw std::runtime_error{ "position outside of string" }; }
      else if(count == npos || s.length < pos + count)
      { count = s.length - pos; }

      if(count <= local_capacity)
      {
        ptr.p = local_buf;
        length = count;
        memcpy(local_buf, s.ptr.p + pos, count);
        local_buf[length] = 0;
      }
      else
      {
        ptr.p = s.ptr.p + pos;
        length = count;
        /* Not null-terminated! */
      }
    }

    constexpr gc_string(native_string_view const &s)
      : gc_string{ s.data(), s.size() }
    { }

    constexpr gc_string(std::string const &s)
      : gc_string{ s.data(), s.size() }
    { }

    constexpr ~gc_string()
    { dispose(); }

    /*** Data accessors. ***/
    constexpr size_type size() const
    { return length; }
    constexpr size_type capacity() const
    { return is_local() ? local_capacity : allocated_capacity; }
    constexpr size_type empty() const
    { return length == 0; }
    /* This may not be null-terminated! */
    constexpr const_pointer_type data() const
    { return ptr.p; }

    /*** Searches. ***/
    constexpr size_type find(value_type const *pattern, size_type const pos = 0) const
    {
      auto const pattern_length(traits_type::length(pattern));
      if(pattern_length == 0)
      { return pos <= length ? pos : npos; }
      else if(length <= pos)
      { return npos; }

      auto const pattern_start(pattern[0]);
      const_pointer_type corpos_pos(ptr.p + pos);
      const_pointer_type const corpus_last(ptr.p + length);
      auto remaining_len(length - pos);

      while(remaining_len >= pattern_length)
      {
        corpos_pos = traits_type::find(corpos_pos, remaining_len - pattern_length + 1, pattern_start);
        if(!corpos_pos)
        { return npos; }

        /* We compare the full string here, including the first character which we've
         * already matched, since the pattern is likely aligned and comparing from the start
         * will be faster for memcmp. */
        if(traits_type::compare(corpos_pos, pattern, pattern_length) == 0)
        { return corpos_pos - ptr.p; }
        remaining_len = corpus_last - ++corpos_pos;
      }
      return npos;
    }

    /*** Modifications. ***/
    constexpr gc_string substr(size_type const pos = 0, size_type const count = npos) const
    { return { *this, pos, count }; }

    /*** Comparisons. ***/
    constexpr native_bool operator !=(gc_string const &s) const
    { return length != s.length || traits_type::compare(ptr.p, s.ptr.p, length); }
    constexpr native_bool operator ==(gc_string const &s) const
    { return !(*this != s); }

    constexpr native_bool operator !=(const_pointer_type const s) const
    { return length != traits_type::length(s) || traits_type::compare(ptr.p, s, length); }
    constexpr native_bool operator ==(const_pointer_type const s) const
    { return !(*this != s); }

    constexpr native_integer to_hash() const
    {
      if(hash != 0)
      { return hash; }

      /* https://github.com/openjdk/jdk/blob/7e30130e354ebfed14617effd2a517ab2f4140a5/src/java.base/share/classes/java/lang/StringLatin1.java#L194 */
      native_integer h{};
      for (size_type i{}; i != length; ++i)
      { h = 31 * h + (ptr.p[i] & 0xff); }
      hash = h;
      return h;
    }

    constexpr operator native_string_view() const
    { return { ptr.p, length }; }

  private:
    constexpr void construct
    (const_pointer_type const begin, const_pointer_type const end, std::forward_iterator_tag)
    {
      auto len(static_cast<size_type>(std::distance(begin, end)));

      if(len > local_capacity)
      {
        ptr.p = create(len, 0);
        allocated_capacity = len;
      }
      else
      { use_local_data(); }

      traits_type::copy(ptr.p, begin, len);

      set_length(len);
    }

    constexpr size_type max_size() const noexcept
    { return (allocator_traits::max_size(ptr) - 1) / 2; }

    constexpr pointer_type create(size_type &capacity, size_type const old_capacity)
    {
      auto const ms(max_size());
      if(capacity > ms) [[unlikely]]
      { throw std::runtime_error{ "string is too big" }; }

      if(capacity > old_capacity && capacity < 2 * old_capacity)
      {
        capacity = 2 * old_capacity;
        if(capacity > ms) [[unlikely]]
        { capacity = ms; }
      }

      return allocator_traits::allocate(ptr, capacity + 1);
    }

    constexpr void use_local_data() noexcept
    {
      if(std::is_constant_evaluated())
      {
        for(size_type i{}; i <= local_capacity; ++i)
        { local_buf[i] = value_type{}; }
      }
    }

    constexpr void set_length(size_type const n) noexcept
    {
      traits_type::assign(ptr.p[n], value_type{});
      length = n;
    }

    constexpr native_bool is_local() const
    { return ptr.p == local_buf; }

    constexpr void dispose()
    {
      if(!is_local() && ptr.p[length] == 0)
      { destroy(allocated_capacity); }
    }

    constexpr void destroy(size_type const size) noexcept
    { allocator_traits::deallocate(ptr, ptr.p, size + 1); }

    // Use empty-base optimization: http://www.cantrip.org/emptyopt.html
    struct alloc_hider : allocator_type
    {
      alloc_hider() = default;
      constexpr alloc_hider(pointer_type const dat, allocator_type const &a)
        : allocator_type{ a }, p{ dat }
      { }

      constexpr alloc_hider(pointer_type const dat, allocator_type &&a = allocator_type{})
        : allocator_type{ std::move(a) }, p{ dat }
      { }

      pointer_type p;
    };

    alloc_hider	ptr;
    size_type length;

    union
    {
      value_type local_buf[local_capacity + 1];
      size_type allocated_capacity;
    };

    mutable native_integer hash{};
  };

  inline std::ostream& operator<<(std::ostream &os, gc_string const &s)
  {
    os.write(s.data(), static_cast<std::streamsize>(s.size()));
    return os;
  }
}
