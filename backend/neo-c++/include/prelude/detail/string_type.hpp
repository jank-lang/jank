#pragma once

#include <ostream>
#include <memory>

#include <immer/box.hpp>
#include <immer/memory_policy.hpp>

namespace jank::detail
{
  template <typename MP>
  struct string_type_impl
  {
    using value_type = immer::box<std::string, MP>;

    string_type_impl() = default;
    string_type_impl(string_type_impl const &s)
      : data{ s.data }
      , length{ s.length }
      , hash{ s.hash }
    { }
    string_type_impl(string_type_impl &&s)
      : data{ std::move(s.data) }
      , length{ s.length }
      , hash{ s.hash }
    { }
    template <size_t N>
    string_type_impl(char const (&s)[N])
      : data{ s }
      , length{ N }
    { }
    string_type_impl(std::string const &s)
      /* Add one for the trailing null. */
      : data{ s }
      , length{ s.size() }
    { }

    bool operator==(string_type_impl const &s) const
    { return to_hash() == s.to_hash(); }

    size_t to_hash() const
    {
      if(hash != 0)
      { return hash; }

      /* https://github.com/openjdk/jdk/blob/7e30130e354ebfed14617effd2a517ab2f4140a5/src/java.base/share/classes/java/lang/StringLatin1.java#L194 */
      auto const &s(data.get());
      size_t h{};
      for(size_t i{}; i < length; ++i)
      { h = 31 * h + (s[i] & 0xff); }
      const_cast<string_type_impl*>(this)->hash = h;
      return h;
    }

    value_type data;
    /* TODO: Consider removing this. */
    size_t length{};
    size_t hash{};

    template <typename M>
    friend std::ostream& operator<<(std::ostream&, string_type_impl<M> const&);
  };

  template <typename MP>
  string_type_impl<MP> operator+(string_type_impl<MP> const &l, string_type_impl<MP> const &r)
  { return { l.data.get() + r.data.get() }; }

  template <typename MP>
  std::ostream& operator<<(std::ostream &os, string_type_impl<MP> const &s)
  { return os << s.data.get(); }
}
