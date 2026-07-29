#include <jtl/immutable_string.hpp>

namespace jtl
{
  struct utf8_iterator
  {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = immutable_string;
    using reference = value_type const &;

    utf8_iterator() = default;
    utf8_iterator(immutable_string const &s);
    utf8_iterator(immutable_string const &s, value_type::size_type const position);

    value_type operator*() const;
    utf8_iterator &operator++();
    utf8_iterator operator++(int);
    utf8_iterator &operator--();
    utf8_iterator operator--(int);
    bool operator==(utf8_iterator const &it) const;

    value_type data;
    value_type::size_type i{};
    value_type::size_type n{};
  };

  struct utf8_range
  {
    utf8_range(immutable_string const &s);

    utf8_iterator begin() const;
    utf8_iterator end() const;

    immutable_string data;
  };

  jtl::immutable_string to_char(i64 const ch);
  jtl::immutable_string to_char(i64 const ch, jtl::immutable_string const &fallback);
  bool is_surrogate_high(u16 const high);
  bool is_surrogate_low(u16 const low);
  u32 combine_surrogate_pair(u16 const high, u16 const low);
  immutable_string::size_type
  next_char_size(immutable_string const &s, immutable_string::size_type const i);
}
