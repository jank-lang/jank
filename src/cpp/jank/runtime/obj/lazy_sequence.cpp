#include "jank/runtime/obj/persistent_list.hpp"
#include <jank/runtime/obj/lazy_sequence.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::runtime
{
  obj::lazy_sequence::static_object(object_ptr const fn)
    : fn{ fn }
  {
    assert(fn);
  }

  obj::lazy_sequence::static_object(object_ptr const fn, object_ptr const sequence)
    : fn{ fn }
    , sequence{ sequence }
  {
  }

  obj::lazy_sequence_ptr obj::lazy_sequence::seq() const
  {
    resolve_seq();
    return sequence ? this : nullptr;
  }

  obj::lazy_sequence_ptr obj::lazy_sequence::fresh_seq() const
  {
    auto ret(make_box<obj::lazy_sequence>(fn, sequence));
    ret->fn_result = fn_result;
    return ret;
  }

  object_ptr obj::lazy_sequence::first() const
  {
    resolve_seq();
    if(sequence)
    {
      return runtime::first(sequence);
    }
    return obj::nil::nil_const();
  }

  obj::lazy_sequence_ptr obj::lazy_sequence::next() const
  {
    resolve_seq();
    if(sequence)
    {
      auto const n(runtime::next(sequence));
      if(n == obj::nil::nil_const())
      {
        return nullptr;
      }
      return make_box<obj::lazy_sequence>(nullptr, n);
    }
    return nullptr;
  }

  obj::lazy_sequence_ptr obj::lazy_sequence::next_in_place()
  {
    resolve_seq();
    if(sequence)
    {
      /* We don't know we own this sequence, so we can't use next_in_place. */
      sequence = runtime::next(sequence);
      if(sequence == obj::nil::nil_const())
      {
        sequence = nullptr;
      }
      return sequence ? this : nullptr;
    }
    return nullptr;
  }

  object_ptr obj::lazy_sequence::next_in_place_first()
  {
    next_in_place();
    if(sequence)
    {
      return runtime::first(sequence);
    }
    return obj::nil::nil_const();
  }

  native_bool obj::lazy_sequence::equal(object const &o) const
  {
    return visit_object(
      [this](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!behavior::seqable<T>)
        {
          return false;
        }
        else
        {
          auto seq(typed_o->fresh_seq());
          for(auto it(fresh_seq()); it != nullptr;
              it = it->next_in_place(), seq = seq->next_in_place())
          {
            if(seq == nullptr || !runtime::detail::equal(it, seq->first()))
            {
              return false;
            }
          }
          return true;
        }
      },
      &o);
  }

  void obj::lazy_sequence::to_string(fmt::memory_buffer &buff) const
  {
    runtime::detail::to_string(seq(), buff);
  }

  native_persistent_string obj::lazy_sequence::to_string() const
  {
    return runtime::detail::to_string(seq());
  }

  native_hash obj::lazy_sequence::to_hash() const
  {
    auto const s(seq());
    if(!s)
    {
      return 1;
    }
    return hash::ordered(s);
  }

  obj::cons_ptr obj::lazy_sequence::cons(object_ptr const head) const
  {
    resolve_seq();
    return make_box<obj::cons>(head, sequence ? this : nullptr);
  }

  object_ptr obj::lazy_sequence::resolve_fn() const
  {
    if(fn)
    {
      fn_result = dynamic_call(fn);
      fn = nullptr;
      if(fn_result == obj::nil::nil_const())
      {
        fn_result = nullptr;
      }
    }
    if(fn_result)
    {
      return fn_result;
    }
    return sequence;
  }

  object_ptr obj::lazy_sequence::resolve_seq() const
  {
    resolve_fn();
    if(fn_result)
    {
      object_ptr lazy{ fn_result };
      fn_result = nullptr;
      while(lazy && lazy->type == object_type::lazy_sequence)
      {
        lazy = expect_object<obj::lazy_sequence>(lazy)->resolve_fn();
      }
      if(lazy)
      {
        sequence = runtime::seq(lazy);
        if(sequence == obj::nil::nil_const())
        {
          sequence = nullptr;
        }
      }
    }
    return sequence;
  }
}
