#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::behavior
{
  native_box<consable> sequence::cons(object_ptr head) const
  { return jank::make_box<obj::cons>(head, const_cast<sequence_ptr>(this)); }

  behavior::seqable const* sequence::as_seqable() const
  { return this; }

  native_bool sequence::equal(object const &o) const
  {
    auto const *o_seqable(o.as_seqable());
    if(!o_seqable)
    { return false; }

    auto o_seq(o_seqable->seq());
    /* TODO: We already have a seq for this; use that? */
    auto this_seq(seq());
    while(this_seq != nullptr && o_seq != nullptr)
    {
      if(!this_seq->first()->equal(*o_seq->first()))
      { return false; }

      this_seq = this_seq->next_in_place();
      o_seq = o_seq->next_in_place();
    }
    return true;
  }

  native_integer sequence::to_hash() const
  {
    if(hash != 0)
    { return static_cast<native_integer>(hash); }

    for(auto it(this); it != nullptr; it = it->next())
    { hash = runtime::detail::hash_combine(hash, *it->first()); }

    return static_cast<native_integer>(hash);
  }
}
