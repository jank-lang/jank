#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::runtime::obj
{
  iterator::iterator(behavior::callable_ptr const fn, object_ptr const start)
    : fn{ fn }, current{ start }
  { }

  behavior::sequence_ptr iterator::seq() const
  { return static_cast<sequence_ptr>(const_cast<iterator*>(this)); }

  object_ptr iterator::first() const
  { return current; }

  behavior::sequence_ptr iterator::next() const
  {
    if(cached_next)
    { return cached_next; }

    auto const next(fn->call(current));
    /* TODO: Why can't I use make_box here? */
    auto const ret(new (GC) iterator{ fn, next });
    cached_next = ret;

    return ret;
  }

  behavior::sequence_ptr iterator::next_in_place()
  {
    if(cached_next)
    { return cached_next; }

    auto const next(fn->call(current));
    current = next;

    return this;
  }

  object_ptr iterator::next_in_place_first()
  {
    if(cached_next)
    { return cached_next; }

    auto const next(fn->call(current));
    current = next;

    return current;
  }

  void iterator::to_string(fmt::memory_buffer &buff) const
  { runtime::detail::to_string(seq(), buff); }
  native_string iterator::to_string() const
  { return runtime::detail::to_string(seq()); }
}
