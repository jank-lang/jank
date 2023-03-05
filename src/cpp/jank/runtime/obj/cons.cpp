#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::runtime::obj
{
  cons::cons(object_ptr const head, behavior::sequence_ptr const tail)
    : head{ head }, tail{ tail }
  { }

  behavior::sequence_ptr cons::seq() const
  { return static_cast<sequence_ptr>(const_cast<cons*>(this)); }

  object_ptr cons::first() const
  { return head; }

  behavior::sequence_ptr cons::next() const
  { return tail; }

  behavior::sequence_ptr cons::next_in_place()
  {
    if(!tail)
    { return nullptr; }

    head = tail->first();
    /* TODO: Can this be in place? */
    tail = tail->next();

    return this;
  }

  object_ptr cons::next_in_place_first()
  {
    if(!tail)
    { return nullptr; }

    head = tail->first();
    /* TODO: Can this be in place? */
    tail = tail->next();

    return head;
  }

  void cons::to_string(fmt::memory_buffer &buff) const
  { runtime::detail::to_string(seq(), buff); }
  native_string cons::to_string() const
  { return runtime::detail::to_string(seq()); }
  native_integer cons::to_hash() const
  {
    if(hash != 0)
    { return static_cast<native_integer>(hash); }

    /* TODO: Do this without going through the seq twice? */
    hash = 1 + runtime::detail::sequence_length(tail);
    auto next(tail->next());
    hash = runtime::detail::hash_combine(hash, next);
    for(auto i(next->next()); i != nullptr; i = i->next_in_place())
    { hash = runtime::detail::hash_combine(hash, i); }
    return static_cast<native_integer>(hash);
  }
}
