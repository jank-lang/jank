#pragma once

#include <ostream>
#include <memory>

#include <immer/box.hpp>
#include <immer/memory_policy.hpp>

#include <jank/option.hpp>

namespace jank::runtime::detail
{
  template <typename T>
  struct list_node
  {
    static constexpr native_bool pointer_free{ false };

    list_node() = default;
    list_node(list_node const &) = default;
    list_node(list_node &&) noexcept = default;

    list_node(T const &t)
      : first{ t }
    {
    }

    list_node(T &&t)
      : first{ std::move(t) }
      , length{ 1 }
    {
    }

    list_node(T const &t, native_box<list_node<T>> const &r, size_t const s)
      : first{ t }
      , rest{ r }
      , length{ s + 1 }
    {
    }

    list_node(T &&t, native_box<list_node<T>> const &r, size_t const s)
      : first{ std::move(t) }
      , rest{ r }
      , length{ s + 1 }
    {
    }

    T first;
    /* TODO: This should ultimately be able to point to anything which
     * implements the equivalent of IPersistentList. */
    native_box<list_node<T>> rest;
    size_t length{};
  };

  template <typename T>
  struct native_persistent_list_impl
  {
    using element_type = T;
    using value_type = list_node<T>;

    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = T;
      using pointer = value_type *;
      using reference = value_type &;

      value_type operator*() const
      {
        if(!latest)
        /* TODO: panic */
        {
          std::abort();
        }
        return latest->first;
      }

      pointer operator->() const
      {
        if(!latest)
        /* TODO: panic */
        {
          std::abort();
        }
        return latest;
      }

      iterator &operator++()
      {
        if(!latest)
        /* TODO: panic */
        {
          std::abort();
        }
        latest = latest->rest;
        return *this;
      }

      native_bool operator==(iterator const &rhs) const
      {
        return latest == rhs.latest;
      }

      native_bool operator!=(iterator const &rhs) const
      {
        return latest != rhs.latest;
      }

      native_box<native_persistent_list_impl<T>::value_type> latest;
    };

    native_persistent_list_impl() = default;
    native_persistent_list_impl(native_persistent_list_impl<T> const &) = default;
    native_persistent_list_impl(native_persistent_list_impl<T> &&) noexcept = default;

    native_persistent_list_impl(native_box<value_type> const &d)
      : data{ d }
    {
    }

    native_persistent_list_impl(std::initializer_list<T> const &vs)
      : native_persistent_list_impl(std::rbegin(vs), std::rend(vs))
    {
    }

    template <typename It>
    native_persistent_list_impl(It const &rb, It const &re)
    {
      size_t length{};
      for(auto it(rb); it != re; ++it)
      {
        data = make_box<value_type>(*it, data, length++);
      }
    }

    native_persistent_list_impl &operator=(native_persistent_list_impl<T> const &) = default;
    native_persistent_list_impl &operator=(native_persistent_list_impl<T> &&) noexcept = default;

    iterator begin() const
    {
      return { data };
    }

    iterator end() const
    {
      return { nullptr };
    }

    native_persistent_list_impl<T> cons(T const &t) const
    {
      return { make_box<value_type>(t, data, size()) };
    }

    native_persistent_list_impl<T> cons(T &&t) const
    {
      return { make_box<value_type>(std::move(t), data, size()) };
    }

    native_persistent_list_impl<T> into(native_persistent_list_impl<T> const &head) const
    {
      if(head.data == nullptr)
      {
        return *this;
      }

      auto tail(head.data);
      while(tail->rest != nullptr)
      {
        tail = tail->rest;
      }
      tail->rest = data;
      return head;
    }

    size_t size() const
    {
      return data ? data->length : 0;
    }

    native_bool empty() const
    {
      return data ? data->length == 0 : true;
    }

    option<T> first() const
    {
      if(data)
      {
        return some(data->first);
      }
      return none;
    }

    native_persistent_list_impl<T> rest() const
    {
      return data ? native_persistent_list_impl<T>{ data->rest } : native_persistent_list_impl<T>{};
    }

    native_box<value_type> data{};
  };

  using native_persistent_list = native_persistent_list_impl<object_ptr>;
}
