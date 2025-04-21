#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime
{
  template <typename T>
  struct sequence_range
  {
    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = object_ref;
      using pointer = object_ref;
      using reference = value_type &;

      iterator() = default;
      iterator(iterator const &) noexcept = default;
      iterator(iterator &&) noexcept = default;

      iterator(object_ref const data)
        : data{ data }
      {
      }

      value_type operator*() const
      {
        return runtime::first(data);
      }

      pointer operator->() const
      {
        return runtime::first(data);
      }

      iterator &operator++()
      {
        data = runtime::next(data);

        return *this;
      }

      bool operator!=(iterator const &rhs) const
      {
        return data != rhs.data;
      }

      bool operator==(iterator const &rhs) const
      {
        return data == rhs.data;
      }

      iterator &operator=(iterator const &) = default;

      object_ref data;
    };

    iterator begin() const
    {
      return { s };
    }

    iterator end() const
    {
      return { jank_nil };
    }

    sequence_range skip(usize const n) const
    {
      sequence_range const r{ s };
      auto it{ r.begin() };
      for(usize i{}; i < n; ++i)
      {
        ++it;
      }

      return { it.data };
    }

    object_ref s;
  };

  template <typename T>
  requires behavior::sequenceable_in_place<T>
  struct sequence_range<T>
  {
    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = object_ref;
      using pointer = object_ref;
      using reference = value_type &;

      iterator() = default;
      iterator(iterator const &) noexcept = default;
      iterator(iterator &&) noexcept = default;

      iterator(oref<T> const data)
        : data{ data }
      {
      }

      auto operator*() const
      {
        return data->first();
      }

      auto operator->() const
      {
        return data->first();
      }

      iterator &operator++()
      {
        data = data->next_in_place();

        return *this;
      }

      bool operator!=(iterator const &rhs) const
      {
        return data != rhs.data;
      }

      bool operator==(iterator const &rhs) const
      {
        return data == rhs.data;
      }

      iterator &operator=(iterator const &) = default;

      oref<T> data;
    };

    iterator begin() const
    {
      return { s };
    }

    iterator end() const
    {
      return { jank_nil };
    }

    sequence_range skip(usize const n) const
    {
      sequence_range const r{ s };
      auto it{ r.begin() };
      for(usize i{}; i < n; ++i)
      {
        ++it;
      }

      return { it.data };
    }

    oref<T> s;
  };

  template <typename T>
  requires behavior::seqable<T>
  auto make_sequence_range(oref<T> const s)
  {
    using S = typename decltype(s->seq())::value_type;

    if constexpr(behavior::sequenceable_in_place<S>)
    {
      return sequence_range<S>{ s.is_some() ? s->fresh_seq() : oref<S>{} };
    }
    else
    {
      return sequence_range<S>{ s.is_some() ? s->seq() : s };
    }
  }
}
