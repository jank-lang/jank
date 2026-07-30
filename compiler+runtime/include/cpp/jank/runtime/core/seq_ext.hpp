#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/sequence_range.hpp>

/* TODO: Why does this not live in seq.hpp again? Document if you find out. */
namespace jank::runtime
{
  template <typename It>
  bool equal(object const &o, It const &begin, It const &end)
  {
    /* nil is seqable, but we don't want it to be equal to an empty collection.
     * An empty seq itself is nil, but that's different. */
    if(o.type == object_type::nil || !o.has_behavior(object_behavior::seqable))
    {
      return false;
    }
    else
    {
      auto const r{ make_sequence_range(detail::untagged(&o)) };
      auto seq_it(r.begin());
      auto it(begin);
      for(; it != end; ++it, ++seq_it)
      {
        if(seq_it == r.end() || !runtime::equal(*it, *seq_it))
        {
          return false;
        }
      }
      return seq_it == r.end() && it == end;
    }
  }

  template <typename T>
  requires behavior::sequence_like<T>
  object_ref rest(oref<T> const &seq)
  {
    if(seq.is_nil())
    {
      return obj::persistent_list::empty();
    }
    auto const ret(seq->next());
    if(ret.is_nil())
    {
      return obj::persistent_list::empty();
    }
    return ret;
  }
}
