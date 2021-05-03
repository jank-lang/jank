#pragma once

#include <ostream>
#include <memory>

namespace jank::detail
{
  struct string_type
  {
    string_type() = default;
    string_type(string_type const &s)
      : data{ std::make_unique<char[]>(s.length) }
      , length{ s.length }
      , hash{ s.hash }
    { std::copy_n(s.data.get(), s.length, data.get()); }
    string_type(string_type &&s)
      : data{ s.data.release() }
      , length{ s.length }
      , hash{ s.hash }
    { }
    template <size_t N>
    string_type(char const (&s)[N])
      : data{ std::make_unique<char[]>(N) }
      , length{ N }
    { std::copy_n(s, N, data.get()); }
    string_type(std::string const &s)
      /* Add one for the trailing null. */
      : data{ std::make_unique<char[]>(s.size() + 1) }
      , length{ s.size() }
    { std::copy_n(s.begin(), s.size(), data.get()); }

    bool operator==(string_type const &s) const
    {
      if(length != s.length)
      { return false; }

      for(size_t i{}; i < length; ++i)
      {
        if(data[i] != s.data[i])
        { return false; }
      }
      return true;
    }

    size_t to_hash() const
    {
      if(hash != 0)
      { return hash; }

      size_t h{};
      for(size_t i{}; i < length; ++i)
      { h = 31 * h + (data[i] & 0xff); }
      const_cast<string_type*>(this)->hash = h;
      return h;
    }

    std::unique_ptr<char[]> data;
    size_t length{};
    size_t hash{};

    friend std::ostream& operator<<(std::ostream&, string_type const&);
  };

  inline string_type operator+(string_type const &l, string_type const &r)
  {
    string_type res;
    res.data = std::make_unique<char[]>(l.length + r.length);
    std::copy_n(l.data.get(), l.length, res.data.get());
    std::copy_n(r.data.get(), r.length, res.data.get()  + l.length);
    return res;
  }

  inline std::ostream& operator<<(std::ostream &os, string_type const &s)
  { return os << s.data.get(); }
}
