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
    utf8_iterator(immutable_string const &s, value_type::size_type const i);

    value_type operator*() const;
    utf8_iterator &operator++();
    utf8_iterator operator++(int);
    utf8_iterator &operator--();
    utf8_iterator operator--(int);
    bool operator==(utf8_iterator const &it) const;

    utf8_iterator begin();
    utf8_iterator end();

    value_type data;
    value_type::size_type i{};
    value_type::size_type n{};
  };
}
