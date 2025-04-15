#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/core/equal.hpp>

namespace jank::runtime
{
  template <typename It>
  native_bool equal(object const &o, It const begin, It const end)
  {
    return visit_seqable(
      [](auto const typed_o, auto const begin, auto const end) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        /* nil is seqable, but we don't want it to be equal to an empty collection.
           An empty seq itself is nil, but that's different. */
        if constexpr(std::same_as<T, obj::nil>)
        {
          return false;
        }
        else
        {
          auto seq(typed_o->fresh_seq());
          auto it(begin);
          for(; it != end; ++it, seq = seq->next_in_place())
          {
            if(seq.is_nil() || !runtime::equal(*it, seq->first()))
            {
              return false;
            }
          }
          return seq.is_nil() && it == end;
        }
      },
      []() { return false; },
      &o,
      begin,
      end);
  }

  template <typename T>
  requires behavior::sequenceable<T>
  auto rest(oref<T> const seq)
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
