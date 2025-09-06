#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  iterator::iterator()
    : object{ obj_type }
  {
  }

  iterator::iterator(object_ref const fn, object_ref const start)
    : object{ obj_type }
    , fn{ fn }
    , current{ start }
  {
  }

  iterator_ref iterator::seq()
  {
    return this;
  }

  iterator_ref iterator::fresh_seq() const
  {
    return make_box<iterator>(fn, current);
  }

  object_ref iterator::first() const
  {
    return current;
  }

  iterator_ref iterator::next() const
  {
    if(cached_next.is_some())
    {
      return cached_next;
    }

    auto const next(dynamic_call(fn, current));
    auto const ret(make_box<iterator>(fn, next));
    cached_next = ret;

    return ret;
  }

  iterator_ref iterator::next_in_place()
  {
    if(cached_next.is_some())
    {
      current = cached_next->first();
      cached_next = jank_nil;
    }
    else
    {
      auto const next(dynamic_call(fn, current));
      current = next;
    }

    return this;
  }

  bool iterator::equal(object const &o) const
  {
    return runtime::sequence_equal(this, &o);
  }

  void iterator::to_string(jtl::string_builder &buff) const
  {
    runtime::to_string(const_cast<iterator *>(this)->seq(), buff);
  }

  jtl::immutable_string iterator::to_string() const
  {
    return runtime::to_string(const_cast<iterator *>(this)->seq());
  }

  jtl::immutable_string iterator::to_code_string() const
  {
    return runtime::to_code_string(const_cast<iterator *>(this)->seq());
  }

  uhash iterator::to_hash() const
  {
    return hash::ordered(this);
  }

  cons_ref iterator::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }
}
