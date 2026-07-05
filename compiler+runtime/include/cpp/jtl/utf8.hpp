#include <jtl/immutable_string.hpp>

namespace jtl
{
  constexpr bool
  is_continuation_byte(immutable_string const &s, immutable_string::size_type const i)
  {
    if(i == immutable_string::npos)
    {
      throw std::runtime_error{ "Invalid unicode string." };
    }

    return (s[i] & 0xC0) == 0x80;
  }

  struct utf8_reverse_range
  {
    utf8_reverse_range(immutable_string const &s)
      : data{ s }
    {
    }

    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = immutable_string;
      using reference = value_type const &;

      iterator()
        : data{}
        , i{}
        , n{}
      {
      }

      iterator(immutable_string const &s)
        : data{ s }
        , i{ s.size() }
        , n{ prev_char_size() }
      {
      }

      constexpr value_type::size_type prev_char_size() const
      {
        if(!is_continuation_byte(data, i - 1))
        {
          return 1;
        }
        else if(!is_continuation_byte(data, i - 2))
        {
          return 2;
        }
        else if(!is_continuation_byte(data, i - 3))
        {
          return 3;
        }
        else if(!is_continuation_byte(data, i - 4))
        {
          return 4;
        }
        else
        {
          throw std::runtime_error{ "Invalid unicode string." };
        }
      }

      value_type operator*() const
      {
        return data.substr(i - n, n);
      }

      iterator &operator++()
      {
        i -= n;
        if(i > n)
        {
          n = prev_char_size();
        }
        return *this;
      }

      bool operator==(iterator const &it) const
      {
        return i == it.i;
      }

      value_type data;
      value_type::size_type i;
      value_type::size_type n;
    };

    iterator begin() const
    {
      if(data.empty())
      {
        return {};
      }

      return { data };
    }

    iterator end() const
    {
      return {};
    }

    immutable_string data;
  };
}
