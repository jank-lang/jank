#pragma once

#include <ostream>
#include <memory>
#include <cstring>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <immer/box.hpp>
#include <immer/memory_policy.hpp>
#pragma clang diagnostic pop

namespace jank::runtime::detail
{
  /* This is a custom string impl with the primary benefit of memoized hashing. */
  template <typename MP>
  struct string_type_impl
  {
    using value_type = immer::box<std::string, MP>;

    string_type_impl() = default;
    string_type_impl(string_type_impl const &s) = default;
    string_type_impl(string_type_impl &&s) = default;
    template <size_t N>
    string_type_impl(char const (&s)[N])
      : data{ s }
      , length{ N }
    { }
    string_type_impl(char const *s)
      : data{ s }
      , length{ std::strlen(s) }
    { }
    string_type_impl(std::string const &s)
      /* Add one for the trailing null. */
      : data{ s }
      , length{ s.size() }
    { }
    string_type_impl(std::string_view const &s)
      : data{ s }
      , length{ s.size() }
    { }
    string_type_impl(char const * const s, size_t const l)
      : data{ s, l }
      , length{ l }
    { }

    string_type_impl<MP>& operator=(string_type_impl<MP> const&) = default;
    string_type_impl<MP>& operator=(string_type_impl<MP> &&) = default;

    bool operator==(string_type_impl const &s) const
    { return to_hash() == s.to_hash(); }

    bool empty() const
    { return length == 0; }

    size_t to_hash() const
    {
      if(hash != 0)
      { return hash; }

      /* https://github.com/openjdk/jdk/blob/7e30130e354ebfed14617effd2a517ab2f4140a5/src/java.base/share/classes/java/lang/StringLatin1.java#L194 */
      auto const &s(data.get());
      size_t h{};
      for(size_t i{}; i < length; ++i)
      { h = 31 * h + (s[i] & 0xff); }
      hash = h;
      return h;
    }

    operator std::string_view() const
    { return { data.get() }; }

    value_type data;
    /* TODO: Consider removing this. */
    size_t length{};
    mutable size_t hash{};

    template <typename M>
    friend std::ostream& operator<<(std::ostream&, string_type_impl<M> const&);
  };

  template <typename MP>
  string_type_impl<MP> operator+(string_type_impl<MP> const &l, string_type_impl<MP> const &r)
  { return { l.data.get() + r.data.get() }; }
  template <typename MP, size_t N>
  string_type_impl<MP> operator+(string_type_impl<MP> const &l, char const (&r)[N])
  { return { l.data.get() + r }; }
  template <typename MP, size_t N>
  string_type_impl<MP> operator+(char const (&l)[N], string_type_impl<MP> const &r)
  { return { l + r.data.get() }; }

  template <typename MP>
  std::ostream& operator<<(std::ostream &os, string_type_impl<MP> const &s)
  { return os << s.data.get(); }
}
