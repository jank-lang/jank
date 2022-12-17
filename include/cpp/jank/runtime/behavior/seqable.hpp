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
    sequence_ptr next() const override
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }
      /* No point allocating a new wrapper if we're the only one referencing this. Just update
       * in place. This can be the difference of thousands of allocations per iteration. */
      if(pool_item_base<basic_iterator_wrapper<It>>::reference_count == 1)
      {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<basic_iterator_wrapper<It>*>(this)->begin = n;
        return pool_item_base<basic_iterator_wrapper<It>>::ptr_from_this();
      }

      return make_box<basic_iterator_wrapper<It>>(coll, n, end);
    }

    object_ptr coll;
    It begin, end;
  };
}
