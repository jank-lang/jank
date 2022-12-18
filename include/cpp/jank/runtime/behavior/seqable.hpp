#pragma once

#include <sstream>

#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/object.hpp>

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
  };
  using sequence_ptr = sequence::sequence_ptr;

  template <typename It>
  struct basic_iterator_wrapper : sequence, pool_item_base<basic_iterator_wrapper<It>>
  {
    basic_iterator_wrapper() = default;
    basic_iterator_wrapper(object_ptr const &c, It const &b, It const &e)
      : coll{ c }, begin{ b }, end { e }
    { }

    detail::string_type to_string() const override
    {
      std::stringstream ss;
      ss << "(";
      for(auto i(begin); i != end; ++i)
      {
        ss << **i;
        auto n(i);
        if(++n != end)
        { ss << " "; }
      }
      ss << ")";
      return ss.str();
    }
    detail::integer_type to_hash() const override
    { return reinterpret_cast<detail::integer_type>(this); }

    behavior::seqable const* as_seqable() const override
    { return this; }
    sequence_ptr seq() const override
    { return pool_item_base<basic_iterator_wrapper<It>>::ptr_from_this(); }

    object_ptr first() const override
    {
      if(begin == end)
      { return JANK_NIL; }
      return *begin;
    }
    sequence_ptr next_in_place() override
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      return basic_iterator_wrapper<It>::ptr_from_this();
    }
    sequence_ptr next() const override
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return make_box<basic_iterator_wrapper<It>>(coll, n, end);
    }

    object_ptr coll;
    It begin, end;
  };
}
