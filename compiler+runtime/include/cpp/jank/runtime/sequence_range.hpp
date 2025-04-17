#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  struct sequence_range
  {
    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = object_ref;
      using pointer = object_ref;
      using reference = value_type &;

      iterator() = delete;
      iterator(iterator const &) noexcept = default;
      iterator(iterator &&) noexcept = default;
      iterator(object_ref data);

      value_type operator*() const;
      pointer operator->() const;
      iterator &operator++();
      bool operator!=(iterator const &rhs) const;
      bool operator==(iterator const &rhs) const;
      iterator &operator=(iterator const &) = default;

      object_ref data;
      bool in_place{};
    };

    iterator begin() const;
    iterator end() const;
    sequence_range skip(usize n) const;

    object_ref s;
  };

  template <typename T>
  sequence_range make_sequence_range(T const s)
  {
    return sequence_range{ s.erase() };
  }
}
