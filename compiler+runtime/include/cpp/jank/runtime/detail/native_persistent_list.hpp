#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::detail
{
  /* TODO: No doubt this can be optimized. */
  template <typename T>
  struct list_node
  {
    static constexpr bool pointer_free{ false };

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

    list_node(T const &t, jtl::ptr<list_node<T>> const &r, usize const s)
      : first{ t }
      , rest{ r }
      , length{ s + 1 }
    {
    }

    list_node(T &&t, jtl::ptr<list_node<T>> const &r, usize const s)
      : first{ std::move(t) }
      , rest{ r }
      , length{ s + 1 }
    {
    }

    T first;
    jtl::ptr<list_node<T>> rest;
    usize length{};
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

      bool operator==(iterator const &rhs) const
      {
        return latest == rhs.latest;
      }

      bool operator!=(iterator const &rhs) const
      {
        return latest != rhs.latest;
      }

      jtl::ptr<native_persistent_list_impl<T>::value_type> latest;
    };

    native_persistent_list_impl() = default;
    native_persistent_list_impl(native_persistent_list_impl<T> const &) = default;
    native_persistent_list_impl(native_persistent_list_impl<T> &&) noexcept = default;

    native_persistent_list_impl(jtl::ptr<value_type> const &d)
      : data{ d }
    {
    }

    native_persistent_list_impl(std::initializer_list<T> const &vs)
      : native_persistent_list_impl(std::rbegin(vs), std::rend(vs))
    {
    }

    template <typename It>
    /* XXX: These must be reverse iterators. */
    native_persistent_list_impl(It const &rb, It const &re)
    {
      usize length{};
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

    native_persistent_list_impl<T> conj(T const &t) const
    {
      return { make_box<value_type>(t, data, size()) };
    }

    native_persistent_list_impl<T> conj(T &&t) const
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

    usize size() const
    {
      return data ? data->length : 0;
    }

    bool empty() const
    {
      return data ? data->length == 0 : true;
    }

    jtl::option<T> first() const
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

    jtl::ptr<value_type> data{};
  };

  using native_persistent_list = native_persistent_list_impl<object_ref>;
}
