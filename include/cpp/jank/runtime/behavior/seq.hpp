#pragma once

#include <jank/runtime/memory_pool.hpp>

namespace jank::runtime
{
  using object_ptr = detail::box_type<struct object>;

  namespace behavior
  {
    /* TODO: Replace this with something more efficient? */
    struct sequence : virtual pool_item_common_base
    {
      using sequence_ptr = detail::box_type<sequence>;

      virtual ~sequence() = default;
      virtual object_ptr first() const = 0;
      /* TODO: This should be optional. */
      virtual sequence_ptr next() const = 0;
    };
    using sequence_ptr = sequence::sequence_ptr;

    struct seqable
    {
      virtual ~seqable() = default;
      virtual sequence_ptr seq() const = 0;
    };

    /* TODO: Optimize this. */
    template <typename It>
    struct basic_iterator_wrapper : sequence, pool_item_base<basic_iterator_wrapper<It>>
    {
      basic_iterator_wrapper() = default;
      basic_iterator_wrapper(It const &b, It const &e)
        : begin{ b }, end { e }
      { }

      object_ptr first() const override
      { return *begin; }
      sequence_ptr next() const override
      {
        auto n(begin);
        ++n;

        if(n == end)
        { return nullptr; }

        return make_box<basic_iterator_wrapper<It>>(n, end);
      }

      It begin, end;
    };
  }
}
