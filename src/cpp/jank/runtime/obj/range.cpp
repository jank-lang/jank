#include <jank/runtime/obj/range.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/runtime/obj/number.hpp>

namespace jank::runtime::obj
{
  range::range(object_ptr const end)
    : start{ jank::runtime::make_box(0) }, end{ end }, step{ jank::runtime::make_box(1) }
  { }
  range::range(object_ptr const start, object_ptr const end)
    : start{ start }, end{ end }, step{ jank::runtime::make_box(1) }
  { }
  range::range(object_ptr const start, object_ptr const end, object_ptr const step)
    : start{ start }, end{ end }, step{ step }
  { }

  behavior::sequence_ptr range::seq() const
  { return static_cast<sequence_ptr>(const_cast<range*>(this)); }

  object_ptr range::first() const
  { return start; }

  behavior::sequence_ptr range::next() const
  {
    if(cached_next)
    { return cached_next; }

    auto next_start(add(start, step));
    if(!lt(next_start, end))
    { return nullptr; }

    /* TODO: Why can't I use make_box here? */
    auto const ret(new (GC) range{ next_start, end, step });
    return ret;
  }

  behavior::sequence_ptr range::next_in_place()
  {
    if(cached_next)
    { return cached_next; }

    auto next_start(add(start, step));
    if(!lt(next_start, end))
    { return nullptr; }

    start = next_start;

    return this;
  }

  object_ptr range::next_in_place_first()
  {
    if(cached_next)
    { return cached_next; }

    auto next_start(add(start, step));
    if(!lt(next_start, end))
    { return nullptr; }

    start = next_start;

    return start;
  }

  void range::to_string(fmt::memory_buffer &buff) const
  { runtime::detail::to_string(seq(), buff); }
  native_string range::to_string() const
  { return runtime::detail::to_string(seq()); }
}
