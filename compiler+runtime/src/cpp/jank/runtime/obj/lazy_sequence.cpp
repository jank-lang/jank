#include <jank/runtime/obj/lazy_sequence.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>

namespace jank::runtime::obj
{
  lazy_sequence::lazy_sequence(object_ptr const fn)
    : fn{ fn }
  {
    assert(fn);
  }

  lazy_sequence::lazy_sequence(object_ptr const fn, object_ptr const sequence)
    : fn{ fn }
    , sequence{ sequence }
  {
  }

  lazy_sequence_ptr lazy_sequence::seq() const
  {
    resolve_seq();
    return sequence ? this : nullptr;
  }

  lazy_sequence_ptr lazy_sequence::fresh_seq() const
  {
    resolve_seq();
    if(!sequence)
    {
      return nullptr;
    }
    auto const s(runtime::fresh_seq(sequence));
    assert(s != nil::nil_const());
    return make_box<lazy_sequence>(nullptr, s);
  }

  object_ptr lazy_sequence::first() const
  {
    resolve_seq();
    if(sequence)
    {
      return runtime::first(sequence);
    }
    return nil::nil_const();
  }

  lazy_sequence_ptr lazy_sequence::next() const
  {
    resolve_seq();
    if(sequence)
    {
      auto const n(runtime::next(sequence));
      if(n == nil::nil_const())
      {
        return nullptr;
      }
      return make_box<lazy_sequence>(nullptr, n);
    }
    return nullptr;
  }

  lazy_sequence_ptr lazy_sequence::next_in_place()
  {
    assert(sequence);
    auto const n(runtime::next_in_place(sequence));
    if(n == nil::nil_const())
    {
      return nullptr;
    }
    sequence = n;
    return this;
  }

  native_bool lazy_sequence::equal(object const &o) const
  {
    return sequence_equal(this, &o);
  }

  void lazy_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string lazy_sequence::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string lazy_sequence::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash lazy_sequence::to_hash() const
  {
    auto const s(seq());
    if(!s)
    {
      return 1;
    }
    return hash::ordered(s);
  }

  cons_ptr lazy_sequence::conj(object_ptr const head) const
  {
    resolve_seq();
    return make_box<cons>(head, sequence ? this : nullptr);
  }

  object_ptr lazy_sequence::resolve_fn() const
  {
    if(fn)
    {
      fn_result = dynamic_call(fn);
      fn = nullptr;
      if(fn_result == nil::nil_const())
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

  object_ptr lazy_sequence::resolve_seq() const
  {
    resolve_fn();
    if(fn_result)
    {
      object_ptr lazy{ fn_result };
      fn_result = nullptr;
      while(lazy && lazy->type == object_type::lazy_sequence)
      {
        lazy = expect_object<lazy_sequence>(lazy)->resolve_fn();
      }
      if(lazy)
      {
        sequence = runtime::seq(lazy);
        if(sequence == nil::nil_const())
        {
          sequence = nullptr;
        }
      }
    }
    return sequence;
  }

  lazy_sequence_ptr lazy_sequence::with_meta(object_ptr const m) const
  {
    resolve_seq();
    auto const ret(make_box<lazy_sequence>(nullptr, sequence));
    auto const meta(behavior::detail::validate_meta(m));
    ret->meta = meta;
    return ret;
  }
}
