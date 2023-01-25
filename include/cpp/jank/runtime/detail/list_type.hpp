#pragma once

#include <ostream>
#include <memory>

#include <immer/box.hpp>
#include <immer/memory_policy.hpp>

#include <jank/option.hpp>
#include <jank/runtime/hash.hpp>

namespace jank::runtime::detail
{
  template <typename T>
  struct list_node
  {
    list_node() = default;
    list_node(list_node const &) = default;
    list_node(list_node &&) noexcept = default;
    list_node(T const &t) : first{ t }
    { }
    list_node(T &&t) : first{ std::move(t) }, length{ 1 }
    { }
    list_node(T const &t, native_box<list_node<T>> const &r, size_t const s)
      : first{ t }, rest{ r }, length{ s + 1 }
    { }
    list_node(T &&t, native_box<list_node<T>> const &r, size_t const s)
      : first{ std::move(t) }, rest{ r }, length{ s + 1 }
    { }

    T first;
    /* TODO: This should ultimately be able to point to anything which
     * implements the equivalent of IPersistentList. */
    native_box<list_node<T>> rest;
    size_t length{};
  };

  template <typename T>
  struct list_type_impl
  {
    using element_type = T;
    using value_type = list_node<T>;

    struct iterator
    {
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = T;
      using pointer = value_type*;
      using reference = value_type&;

      value_type operator *() const
      {
        if(!latest)
        /* TODO: panic */
        { std::abort(); }
        return latest->first;
      }
      pointer operator ->() const
      {
        if(!latest)
        /* TODO: panic */
        { std::abort(); }
        return latest;
      }
      iterator& operator ++()
      {
        if(!latest)
        /* TODO: panic */
        { std::abort(); }
        latest = latest->rest;
        return *this;
      }
      bool operator ==(iterator const &rhs) const
      { return latest == rhs.latest; }
      bool operator !=(iterator const &rhs) const
      { return latest != rhs.latest; }

      native_box<list_type_impl<T>::value_type> latest;
    };

    list_type_impl() = default;
    list_type_impl(list_type_impl<T> const &) = default;
    list_type_impl(list_type_impl<T> &&) noexcept = default;
    list_type_impl(native_box<value_type> const &d) : data{ d }
    { }

    list_type_impl(std::initializer_list<T> const &vs)
      : list_type_impl(std::rbegin(vs), std::rend(vs))
    { }
    template <typename It>
    list_type_impl(It const &rb, It const &re)
    {
      size_t length{};
      for(auto it(rb); it != re; ++it)
      { data = make_box<value_type>(*it, data, length++); }
    }

    list_type_impl& operator=(list_type_impl<T> const &) = default;
    list_type_impl& operator=(list_type_impl<T> &&) noexcept = default;

    iterator begin() const
    { return { data }; }
    iterator end() const
    { return { nullptr }; }

    list_type_impl<T> cons(T const &t) const
    { return { make_box<value_type>(t, data, size()) }; }
    list_type_impl<T> cons(T &&t) const
    { return { make_box<value_type>(std::move(t), data, size()) }; }

    list_type_impl<T> into(list_type_impl<T> const &head) const
    {
      if(head.data == nullptr)
      { return *this; }

      auto tail(head.data);
      while(tail->rest != nullptr)
      { tail = tail->rest; }
      tail->rest = data;
      return head;
    }

    size_t size() const
    { return data ? data->length : 0; }

    option<T> first() const
    {
      if(data)
      { return some(data->first); }
      return none;
    }
    list_type_impl<T> rest() const
    { return data ? list_type_impl<T>{ data->rest } : list_type_impl<T>{}; }

    native_box<value_type> data{};
  };
}
