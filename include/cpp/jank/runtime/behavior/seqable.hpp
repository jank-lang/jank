#pragma once

#include <jank/native_box.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/consable.hpp>

namespace jank::runtime::obj
{ struct cons; }

namespace jank::runtime::behavior
{
  struct seqable
  {
    virtual ~seqable() = default;
    virtual native_box<struct sequence> seq() const = 0;
    virtual native_box<struct sequence> fresh_seq() const = 0;
  };
  using seqable_ptr = native_box<seqable const>;

  struct sequence : virtual object, seqable, consable
  {
    using sequence_ptr = native_box<sequence>;

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
    native_box<consable> cons(object_ptr head) const final;

    behavior::seqable const* as_seqable() const final;
    native_bool equal(object const &o) const final;
    native_integer to_hash() const override;

    mutable size_t hash{};
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
  struct basic_iterator_wrapper : sequence, countable
  {
    static constexpr bool pointer_free{ false };

    basic_iterator_wrapper() = default;
    basic_iterator_wrapper(object_ptr const &c, It const &b, It const &e, size_t const s)
      : coll{ c }, begin{ b }, end{ e }, size{ s }
    {
      if(begin == end)
      { throw std::runtime_error{ "basic_iterator_wrapper for empty sequence" }; }
    }

    void to_string(fmt::memory_buffer &buff) const final
    { return detail::to_string(begin, end, '(', ')', buff); }
    native_string to_string() const final
    {
      fmt::memory_buffer buff;
      detail::to_string(begin, end, '(', ')', buff);
      return native_string{ buff.data(), buff.size() };
    }
    native_integer to_hash() const final
    { return reinterpret_cast<native_integer>(this); }

    sequence_ptr seq() const final
    { return static_cast<sequence_ptr>(const_cast<basic_iterator_wrapper<It>*>(this)); }
    sequence_ptr fresh_seq() const final
    { return jank::make_box<basic_iterator_wrapper<It>>(coll, begin, end, size); }

    behavior::countable const* as_countable() const final
    { return this; }
    size_t count() const final
    { return size; }

    object_ptr first() const final
    { return *begin; }
    sequence_ptr next() const final
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return jank::make_box<basic_iterator_wrapper<It>>(coll, n, end, size);
    }
    sequence_ptr next_in_place() final
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      return this;
    }
    object_ptr next_in_place_first() final
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      return *begin;
    }

    object_ptr coll{};
    It begin, end;
    size_t size{};
  };

  template <size_t N>
  struct array_sequence : sequence, countable
  {
    static constexpr bool pointer_free{ false };

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
    template <typename ...Args>
    array_sequence(object_ptr first, Args ... rest)
      : arr{ first, rest... }
    { }

    void to_string(fmt::memory_buffer &buff) const final
    { return detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff); }
    native_string to_string() const final
    {
      fmt::memory_buffer buff;
      detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff);
      return native_string{ buff.data(), buff.size() };
    }
    native_integer to_hash() const final
    { return reinterpret_cast<native_integer>(this); }

    sequence_ptr seq() const final
    { return static_cast<sequence_ptr>(const_cast<array_sequence*>(this)); }
    sequence_ptr fresh_seq() const final
    { return jank::make_box<array_sequence<N>>(arr, index); }

    behavior::countable const* as_countable() const final
    { return this; }
    size_t count() const final
    { return N; }

    object_ptr first() const final
    { return arr[index]; }
    sequence_ptr next() const final
    {
      auto n(index);
      ++n;

      if(n == N)
      { return nullptr; }

      return jank::make_box<array_sequence<N>>(arr, n);
    }
    sequence_ptr next_in_place() final
    {
      ++index;

      if(index == N)
      { return nullptr; }

      return this;
    }
    object_ptr next_in_place_first() final
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
  struct vector_sequence : sequence, countable
  {
    static constexpr bool pointer_free{ false };

    vector_sequence() = default;
    vector_sequence(native_vector<object_ptr> const &arr, size_t const index)
      : arr{ arr }, index{ index }
    { }
    vector_sequence(native_vector<object_ptr> &&arr, size_t const index)
      : arr{ std::move(arr) }, index{ index }
    { }
    vector_sequence(native_vector<object_ptr> &&arr)
      : arr{ std::move(arr) }
    { }

    void to_string(fmt::memory_buffer &buff) const final
    { return detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff); }
    native_string to_string() const final
    {
      fmt::memory_buffer buff;
      detail::to_string(arr.begin() + index, arr.end(), '(', ')', buff);
      return native_string{ buff.data(), buff.size() };
    }
    native_integer to_hash() const final
    { return reinterpret_cast<native_integer>(this); }

    sequence_ptr seq() const final
    { return static_cast<sequence_ptr>(const_cast<vector_sequence*>(this)); }
    sequence_ptr fresh_seq() const final
    { return jank::make_box<vector_sequence>(arr, index); }

    behavior::countable const* as_countable() const final
    { return this; }
    size_t count() const final
    { return arr.size(); }

    object_ptr first() const final
    { return arr[index]; }
    sequence_ptr next() const final
    {
      auto n(index);
      ++n;

      if(n == arr.size())
      { return nullptr; }

      return jank::make_box<vector_sequence>(arr, n);
    }
    sequence_ptr next_in_place() final
    {
      ++index;

      if(index == arr.size())
      { return nullptr; }

      return this;
    }
    object_ptr next_in_place_first() final
    {
      ++index;

      if(index == arr.size())
      { return nullptr; }

      return arr[index];
    }

    native_vector<object_ptr> arr;
    size_t index{};
  };
}
