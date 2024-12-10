#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  obj::iterator::static_object(object_ptr const fn, object_ptr const start)
    : fn{ fn }
    , current{ start }
  {
  }

  obj::iterator_ptr obj::iterator::seq()
  {
    return this;
  }

  obj::iterator_ptr obj::iterator::fresh_seq() const
  {
    return make_box<obj::iterator>(fn, current);
  }

  object_ptr obj::iterator::first() const
  {
    return current;
  }

  obj::iterator_ptr obj::iterator::next() const
  {
    if(cached_next)
    {
      return cached_next;
    }

    auto const next(dynamic_call(fn, current));
    auto const ret(make_box<obj::iterator>(fn, next));
    cached_next = ret;

    return ret;
  }

  obj::iterator_ptr obj::iterator::next_in_place()
  {
    if(cached_next)
    {
      current = cached_next->first();
      cached_next = nullptr;
    }
    else
    {
      auto const next(dynamic_call(fn, current));
      current = next;
    }

    return this;
  }

  native_bool obj::iterator::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        for(auto it(fresh_seq()); it != nullptr;
            it = runtime::next_in_place(it), seq = runtime::next_in_place(seq))
        {
          if(seq == nullptr || !runtime::equal(it, seq->first()))
          {
            return false;
          }
        }
        return true;
      },
      []() { return false; },
      &o);
  }

  void obj::iterator::to_string(fmt::memory_buffer &buff)
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string obj::iterator::to_string()
  {
    return runtime::to_string(seq());
  }

  native_persistent_string obj::iterator::to_code_string()
  {
    return runtime::to_code_string(seq());
  }

  native_hash obj::iterator::to_hash() const
  {
    return hash::ordered(&base);
  }

  obj::cons_ptr obj::iterator::conj(object_ptr const head) const
  {
    return make_box<obj::cons>(head, this);
  }
}
