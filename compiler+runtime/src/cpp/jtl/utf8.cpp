#include <jtl/utf8.hpp>

namespace jtl
{
  using value_type = utf8_iterator::value_type;
  using size_type = value_type::size_type;

  static size_type next_char_size(immutable_string const &s, size_type const i)
  {
    auto const c(s[i]);
    if(c <= 0x7f)
    {
      return 1;
    }
    else if((c & 0xE0) == 0xC0)
    {
      return 2;
    }
    else if((c & 0xF0) == 0xE0)
    {
      return 3;
    }
    else if((c & 0xF8) == 0xF0)
    {
      return 4;
    }
    else
    {
      throw std::runtime_error{ "Invalid UTF-8 string." };
    }
  }

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

  utf8_iterator::utf8_iterator(immutable_string const &s)
    : data{ s }
    , i{ s.empty() ? value_type::npos : 0 }
    , n{ i == value_type::npos ? 0 : next_char_size(s, i) }
  {
  }

  utf8_iterator::utf8_iterator(immutable_string const &s, value_type::size_type const i)
    : data{ s }
    , i{ s.empty() ? value_type::npos : i }
    , n{ i == value_type::npos ? 0 : next_char_size(s, i) }
  {
  }

  value_type utf8_iterator::operator*() const
  {
    return data.substr(i, n);
  }

  utf8_iterator &utf8_iterator::operator++()
  {
    if(i == value_type::npos)
    {
      i = 0;
      n = next_char_size(data, i);
    }
    else
    {
      i += n;
      if(i >= data.size())
      {
        i = value_type::npos;
        n = 0;
      }
      else
      {
        n = next_char_size(data, i);
      }
    }

    return *this;
  }

  utf8_iterator utf8_iterator::operator++(int)
  {
    auto tmp{ *this };
    ++*this;
    return tmp;
  }

  utf8_iterator &utf8_iterator::operator--()
  {
    if(i == 0)
    {
      i = value_type::npos;
      n = 0;
    }
    else if(i == value_type::npos)
    {
      auto const size(data.size());
      n = prev_char_size(data, size);
      i = size - n;
    }
    else
    {
      n = prev_char_size(data, i);
      i -= n;
    }
    return *this;
  }

  utf8_iterator utf8_iterator::operator--(int)
  {
    auto tmp{ *this };
    --*this;
    return tmp;
  }

  bool utf8_iterator::operator==(utf8_iterator const &it) const
  {
    return i == it.i && (data.data() == it.data.data() || data == it.data);
  }

  utf8_iterator utf8_iterator::begin() const
  {
    return { data, 0 };
  }

  utf8_iterator utf8_iterator::end() const
  {
    return { data, value_type::npos };
  }
}
