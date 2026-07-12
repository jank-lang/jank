#include <ranges>

#include <jtl/immutable_string.hpp>

namespace jtl
{
  struct reverse_iterator
  {
    using reverse_iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = immutable_string;
    using reference = value_type const &;

    reverse_iterator() = default;
    reverse_iterator(immutable_string const &s);
    reverse_iterator(immutable_string const &s, value_type::size_type const i);

    value_type operator*() const;
    reverse_iterator &operator++();
    reverse_iterator operator++(int);
    bool operator==(reverse_iterator const &it) const;

    value_type data;
    value_type::size_type i{};
    value_type::size_type n{};
  };

  std::ranges::subrange<reverse_iterator> utf8_reverse_range(immutable_string const &s);
}
