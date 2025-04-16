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
      using pointer = value_type *;
      using reference = value_type &;

      iterator() = delete;
      iterator(object_ref data);

      value_type operator*() const;
      pointer operator->();
      iterator &operator++();
      bool operator!=(iterator const &rhs) const;
      bool operator==(iterator const &rhs) const;
      iterator &operator=(iterator const &) = default;

      object_ref data;
    };

    iterator begin();
    iterator end();

    object_ref s;
  };
}
