#include <jtl/utf8.hpp>

namespace jtl
{
  using value_type = reverse_iterator::value_type;
  using size_type = value_type::size_type;

  static bool is_continuation_byte(immutable_string const &s, size_type const i)
  {
    if(i == immutable_string::npos)
    {
      throw std::runtime_error{ "Invalid UTF-8 string." };
    }

    return (s[i] & 0xC0) == 0x80;
  }

  static size_type prev_char_size(immutable_string const &data, size_type const i)
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
      throw std::runtime_error{ "Invalid UTF-8 string." };
    }
  }

  reverse_iterator::reverse_iterator(immutable_string const &s)
    : data{ s }
    , i{ s.size() }
    , n{ prev_char_size(s, i) }
  {
  }

  reverse_iterator::reverse_iterator(immutable_string const &s, value_type::size_type const i)
    : data{ s }
    , i{ i }
    , n{ i == 0 ? 0 : prev_char_size(s, i) }
  {
  }

  value_type reverse_iterator::operator*() const
  {
    return data.substr(i - n, n);
  }

  reverse_iterator &reverse_iterator::operator++()
  {
    i -= n;
    if(i > n)
    {
      n = prev_char_size(data, i);
    }
    return *this;
  }

  reverse_iterator reverse_iterator::operator++(int)
  {
    auto tmp{ *this };
    ++*this;
    return tmp;
  }

  bool reverse_iterator::operator==(reverse_iterator const &it) const
  {
    return data.data() == it.data.data() && i == it.i;
  }

  std::ranges::subrange<reverse_iterator> utf8_reverse_range(immutable_string const &s)
  {
    reverse_iterator end{ s, 0 };
    auto begin(s.empty() ? end : reverse_iterator{ s });
    return { begin, end };
  }

}
