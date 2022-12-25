#pragma once

#include <sstream>

#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/countable.hpp>

namespace jank::runtime::behavior
{
  struct seqable
  {
    virtual ~seqable() = default;
    virtual detail::box_type<struct sequence> seq() const = 0;
  };

  struct sequence : virtual object, seqable
  {
    using sequence_ptr = detail::box_type<sequence>;

    virtual ~sequence() = default;
    virtual object_ptr first() const = 0;
    virtual sequence_ptr next() const = 0;
    /* Each call to next() allocates a new sequence_ptr, since it's polymorphic. When iterating
     * over a large sequence, this can mean a _lot_ of allocations. However, if you own the
     * sequence_ptr you have, typically meaning it wasn't a parameter, then you can mutate it
     * in place using this function. No allocations will happen.
     *
     * If you don't own your sequence_ptr, you can call next() on it once, to get one you
     * do own, and then next_in_place() on that to your heart's content. */
    virtual sequence_ptr next_in_place() = 0;
    virtual object_ptr next_in_place_first() = 0;
  };
  using sequence_ptr = sequence::sequence_ptr;

  namespace detail
  {
    template <typename It>
    void to_string(It const &begin, It const &end, char const open, char const close, fmt::memory_buffer &buff)
    {
      auto inserter(std::back_inserter(buff));
      inserter = open;
      for(auto i(begin); i != end; ++i)
      {
        (*i)->to_string(buff);
        auto n(i);
        if(++n != end)
        { inserter = ' '; }
      }
      inserter = close;
    }
  }

  template <typename It>
  struct basic_iterator_wrapper : sequence, countable, pool_item_base<basic_iterator_wrapper<It>>
  {
    basic_iterator_wrapper() = default;
    basic_iterator_wrapper(object_ptr const &c, It const &b, It const &e, size_t const s)
      : coll{ c }, begin{ b }, end{ e }, size{ s }
    {
      if(begin == end)
      { throw std::runtime_error{ "basic_iterator_wrapper for empty sequence" }; }
    }

    void to_string(fmt::memory_buffer &buff) const override
    { return detail::to_string(begin, end, '(', ')', buff); }
    runtime::detail::string_type to_string() const override
    {
      fmt::memory_buffer buff;
      detail::to_string(begin, end, '(', ')', buff);
      return std::string{ buff.data(), buff.size() };
    }
    runtime::detail::integer_type to_hash() const override
    { return reinterpret_cast<runtime::detail::integer_type>(this); }

    behavior::seqable const* as_seqable() const override
    { return this; }
    sequence_ptr seq() const override
    { return pool_item_base<basic_iterator_wrapper<It>>::ptr_from_this(); }

    behavior::countable const* as_countable() const override
    { return this; }
    size_t count() const override
    { return size; }

    object_ptr first() const override
    { return *begin; }
    sequence_ptr next() const override
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return make_box<basic_iterator_wrapper<It>>(coll, n, end, size);
    }
    sequence_ptr next_in_place() override
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      return basic_iterator_wrapper<It>::ptr_from_this();
    }
    object_ptr next_in_place_first() override
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      return *begin;
    }

    object_ptr coll;
    It begin, end;
    size_t size{};
  };

  template <size_t N>
  struct array_sequence : sequence, countable, pool_item_base<array_sequence<N>>
  {
    array_sequence() = default;
    array_sequence(std::array<object_ptr, N> const &arr, size_t const index)
      : arr{ arr }, index{ index }
    { }
    array_sequence(std::array<object_ptr, N> &&arr, size_t const index)
      : arr{ std::move(arr) }, index{ index }
    { }
    array_sequence(std::array<object_ptr, N> &&arr)
      : arr{ std::move(arr) }
    { }

    void to_string(fmt::memory_buffer &buff) const override
    { return detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff); }
    runtime::detail::string_type to_string() const override
    {
      fmt::memory_buffer buff;
      detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff);
      return std::string{ buff.data(), buff.size() };
    }
    runtime::detail::integer_type to_hash() const override
    { return reinterpret_cast<runtime::detail::integer_type>(this); }

    behavior::seqable const* as_seqable() const override
    { return this; }
    sequence_ptr seq() const override
    { return pool_item_base<array_sequence<N>>::ptr_from_this(); }

    behavior::countable const* as_countable() const override
    { return this; }
    size_t count() const override
    { return N; }

    object_ptr first() const override
    { return arr[index]; }
    sequence_ptr next() const override
    {
      auto n(index);
      ++n;

      if(n == N)
      { return nullptr; }

      return make_box<array_sequence<N>>(arr, n);
    }
    sequence_ptr next_in_place() override
    {
      ++index;

      if(index == N)
      { return nullptr; }

      return array_sequence<N>::ptr_from_this();
    }
    object_ptr next_in_place_first() override
    {
      ++index;

      if(index == N)
      { return nullptr; }

      return arr[index];
    }

    std::array<object_ptr, N> arr;
    size_t index{};
  };

  /* TODO: Move impl to cpp. */
  struct vector_sequence : sequence, countable, pool_item_base<vector_sequence>
  {
    vector_sequence() = default;
    vector_sequence(std::vector<object_ptr> const &arr, size_t const index)
      : arr{ arr }, index{ index }
    { }
    vector_sequence(std::vector<object_ptr> &&arr, size_t const index)
      : arr{ std::move(arr) }, index{ index }
    { }
    vector_sequence(std::vector<object_ptr> &&arr)
      : arr{ std::move(arr) }
    { }

    void to_string(fmt::memory_buffer &buff) const override
    { return detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff); }
    runtime::detail::string_type to_string() const override
    {
      fmt::memory_buffer buff;
      detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff);
      return std::string{ buff.data(), buff.size() };
    }
    runtime::detail::integer_type to_hash() const override
    { return reinterpret_cast<runtime::detail::integer_type>(this); }

    behavior::seqable const* as_seqable() const override
    { return this; }
    sequence_ptr seq() const override
    { return pool_item_base<vector_sequence>::ptr_from_this(); }

    behavior::countable const* as_countable() const override
    { return this; }
    size_t count() const override
    { return arr.size(); }

    object_ptr first() const override
    { return arr[index]; }
    sequence_ptr next() const override
    {
      auto n(index);
      ++n;

      if(n == arr.size())
      { return nullptr; }

      return make_box<vector_sequence>(arr, n);
    }
    sequence_ptr next_in_place() override
    {
      ++index;

      if(index == arr.size())
      { return nullptr; }

      return vector_sequence::ptr_from_this();
    }
    object_ptr next_in_place_first() override
    {
      ++index;

      if(index == arr.size())
      { return nullptr; }

      return arr[index];
    }

    std::vector<object_ptr> arr;
    size_t index{};
  };
}
