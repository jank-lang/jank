#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  iterator::iterator()
    : object{ obj_type, obj_behaviors }
  {
  }

  iterator::iterator(object_ref const fn, object_ref const start)
    : object{ obj_type, obj_behaviors }
    , fn{ fn }
    , current{ start }
  {
  }

  iterator_ref iterator::seq() const
  {
    return runtime::detail::untagged(this);
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

    auto const next(fn.call(current));
    auto const ret(make_box<iterator>(fn, next));
    cached_next.store(reinterpret_cast<iterator *>(ret.raw()));

    return ret;
  }

  iterator_ref iterator::next_in_place()
  {
    iterator_ref const n{ cached_next.load() ?: iterator_ref{} };
    if(n.is_some())
    {
      current = n->first();
      cached_next.store(reinterpret_cast<iterator *>(jank_nil.raw()));
    }
    else
    {
      auto const next(fn.call(current));
      current = next;
    }

    return runtime::detail::untagged(this);
  }

  bool iterator::equal(object const &o) const
  {
    return runtime::sequence_equal(runtime::detail::untagged(this), runtime::detail::untagged(&o));
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
    return hash::ordered(runtime::detail::untagged(this));
  }

  cons_ref iterator::conj(object_ref const head) const
  {
    return make_box<cons>(head, runtime::detail::untagged(this));
  }
}
