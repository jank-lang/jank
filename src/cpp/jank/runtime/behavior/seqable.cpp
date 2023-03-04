#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::behavior
{
  native_box<obj::cons> sequence::cons(object_ptr head) const
  { return jank::make_box<obj::cons>(head, const_cast<sequence_ptr>(this)); }

  behavior::seqable const* sequence::as_seqable() const
  { return this; }

  native_bool sequence::equal(object const &o) const
  {
    auto const *o_seqable(o.as_seqable());
    if(!o_seqable)
    { return false; }

    auto o_seq(o_seqable->seq());
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
}
