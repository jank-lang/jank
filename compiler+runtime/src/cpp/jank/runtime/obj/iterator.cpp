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

  iterator_ref iterator::seq() const
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
    iterator_ref const n{ cached_next.load() ?: iterator_ref{} };
    if(n.is_some())
    {
      return n;
    }

    auto const next(dynamic_call(fn, current));
    auto const ret(make_box<iterator>(fn, next));
    cached_next.store(reinterpret_cast<iterator *>(ret.data));

    return ret;
  }

  iterator_ref iterator::next_in_place()
  {
    iterator_ref const n{ cached_next.load() ?: iterator_ref{} };
    if(n.is_some())
    {
      current = n->first();
      cached_next.store(reinterpret_cast<iterator *>(jank_nil().data));
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
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string iterator::to_string() const
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string iterator::to_code_string() const
  {
    return runtime::to_code_string(seq());
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
