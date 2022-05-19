#pragma once

#include <jank/runtime/detail/type.hpp>

namespace jank::runtime
{
  using object_ptr = detail::box_type<struct object>;

  namespace behavior
  {
    struct sequence : virtual pool_item_common_base
    {
      using sequence_pointer = detail::box_type<sequence>;

      virtual object_ptr first() const = 0;
      virtual sequence_pointer next() const = 0;
    };
    using sequence_pointer = sequence::sequence_pointer;

    struct seqable
    {
      virtual ~seqable() = default;
      virtual sequence_pointer seq() const = 0;
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
      sequence_pointer next() const override
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
