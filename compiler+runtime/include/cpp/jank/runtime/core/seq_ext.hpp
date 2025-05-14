#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/sequence_range.hpp>

/* TODO: Why does this not live in seq.hpp again? Document if you find out. */
namespace jank::runtime
{
  template <typename It>
  bool equal(object const &o, It const begin, It const end)
  {
    return visit_seqable(
      [](auto const typed_o, auto const begin, auto const end) -> bool {
        using T = typename decltype(typed_o)::value_type;

        /* nil is seqable, but we don't want it to be equal to an empty collection.
           An empty seq itself is nil, but that's different. */
        if constexpr(std::same_as<T, obj::nil>)
        {
          return false;
        }
        else
        {
          auto const r{ make_sequence_range(typed_o) };
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
