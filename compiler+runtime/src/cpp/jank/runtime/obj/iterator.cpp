#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  iterator::iterator(object_ptr const fn, object_ptr const start)
    : fn{ fn }
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

  object_ptr iterator::first() const
  {
    return current;
  }

  iterator_ref iterator::next() const
  {
    if(cached_next)
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
    if(cached_next)
    {
      current = cached_next->first();
      cached_next = obj::nil::nil_const();
    }
    else
    {
      auto const next(dynamic_call(fn, current));
      current = next;
    }

    return this;
  }

  native_bool iterator::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        for(auto it(fresh_seq()); it;
            it = it->next_in_place(), seq = seq->next_in_place())
        {
          if(!seq || !runtime::equal(it->first(), seq->first()))
          {
            return false;
          }
        }
        return !seq;
      },
      []() { return false; },
      &o);
  }

  void iterator::to_string(util::string_builder &buff)
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string iterator::to_string()
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string iterator::to_code_string()
  {
    return runtime::to_code_string(seq());
  }

  native_hash iterator::to_hash() const
  {
    return hash::ordered(&base);
  }

  cons_ref iterator::conj(object_ptr const head) const
  {
    return make_box<cons>(head, this);
  }
}
