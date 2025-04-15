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
  lazy_sequence::lazy_sequence(object_ref const fn)
    : fn{ fn }
  {
    jank_debug_assert(fn);
  }

  lazy_sequence::lazy_sequence(object_ref const fn, object_ref const sequence)
    : fn{ fn }
    , sequence{ sequence }
  {
  }

  lazy_sequence_ref lazy_sequence::seq() const
  {
    resolve_seq();
    return sequence.is_some() ? this : lazy_sequence_ref{};
  }

  lazy_sequence_ref lazy_sequence::fresh_seq() const
  {
    resolve_seq();
    if(sequence.is_nil())
    {
      return {};
    }
    auto const s(runtime::fresh_seq(sequence));
    jank_debug_assert(s != nil::nil_const());
    return make_box<lazy_sequence>(nil::nil_const(), s);
  }

  object_ref lazy_sequence::first() const
  {
    resolve_seq();
    if(sequence.is_some())
    {
      return runtime::first(sequence);
    }
    return nil::nil_const();
  }

  lazy_sequence_ref lazy_sequence::next() const
  {
    resolve_seq();
    if(sequence.is_some())
    {
      auto const n(runtime::next(sequence));
      if(n == nil::nil_const())
      {
        return {};
      }
      return make_box<lazy_sequence>(nil::nil_const(), n);
    }
    return {};
  }

  lazy_sequence_ref lazy_sequence::next_in_place()
  {
    jank_debug_assert(sequence);
    auto const n(runtime::next_in_place(sequence));
    if(n == nil::nil_const())
    {
      return {};
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

  jtl::immutable_string lazy_sequence::to_string() const
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string lazy_sequence::to_code_string() const
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
    return hash::ordered(s.erase());
  }

  cons_ref lazy_sequence::conj(object_ref const head) const
  {
    resolve_seq();
    return make_box<cons>(head, sequence.is_some() ? &base : nil::nil_const().erase());
  }

  object_ref lazy_sequence::resolve_fn() const
  {
    if(fn.is_some())
    {
      fn_result = dynamic_call(fn);
      fn = nil::nil_const();
    }
    if(fn_result.is_some())
    {
      return fn_result;
    }
    return sequence;
  }

  object_ref lazy_sequence::resolve_seq() const
  {
    resolve_fn();
    if(fn_result.is_some())
    {
      object_ref lazy{ fn_result };
      fn_result = nil::nil_const();
      while(lazy.is_some() && lazy->type == object_type::lazy_sequence)
      {
        lazy = expect_object<lazy_sequence>(lazy)->resolve_fn();
      }
      if(lazy.is_some())
      {
        sequence = runtime::seq(lazy);
      }
    }
    return sequence;
  }

  lazy_sequence_ref lazy_sequence::with_meta(object_ref const m) const
  {
    resolve_seq();
    auto const ret(make_box<lazy_sequence>(nil::nil_const(), sequence));
    auto const meta(behavior::detail::validate_meta(m));
    ret->meta = meta;
    return ret;
  }
}
