#include <jank/runtime/obj/lazy_sequence.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  lazy_sequence::lazy_sequence(object_ref const fn)
    : fn{ fn }
  {
    jank_debug_assert(fn.is_some());
  }

  lazy_sequence::lazy_sequence(object_ref const fn, object_ref const sequence)
    : fn{ fn }
    , s{ sequence }
  {
  }

  object_ref lazy_sequence::seq() const
  {
    realize();
    return s;
  }

  lazy_sequence_ref lazy_sequence::fresh_seq() const
  {
    realize();
    if(s.is_nil())
    {
      return {};
    }
    auto const r(runtime::fresh_seq(s));
    jank_debug_assert(r != jank_nil);
    return make_box<lazy_sequence>(jank_nil, r);
  }

  object_ref lazy_sequence::first() const
  {
    realize();
    if(s.is_nil())
    {
      return s;
    }
    return runtime::first(s);
  }

  object_ref lazy_sequence::next() const
  {
    realize();
    if(s.is_nil())
    {
      return {};
    }
    auto const n(runtime::next(s));
    return n;
  }

  bool lazy_sequence::equal(object const &o) const
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

  uhash lazy_sequence::to_hash() const
  {
    auto const s(seq());
    if(s.is_nil())
    {
      return 1;
    }
    return hash::ordered(s.erase());
  }

  cons_ref lazy_sequence::conj(object_ref const head) const
  {
    return make_box<cons>(head, seq());
  }

  void lazy_sequence::realize() const
  {
    /* TODO: Lock. */
    force();
    if(sv.is_some())
    {
      auto ls{ sv };
      sv = jank_nil;
      if(ls.is_some() && ls->type == object_type::lazy_sequence)
      {
        ls = unwrap(ls);
      }
      s = runtime::seq(ls);
    }
  }

  void lazy_sequence::force() const
  {
    if(fn.is_some())
    {
      sv = dynamic_call(fn);
      fn = jank_nil;
    }
  }

  void lazy_sequence::lock_and_force() const
  {
    /* TODO: Lock */
    force();
  }

  object_ref lazy_sequence::sval() const
  {
    if(fn.is_some())
    {
      lock_and_force();
    }
    if(sv.is_some())
    {
      return sv;
    }
    return s;
  }

  object_ref lazy_sequence::unwrap(object_ref ls) const
  {
    while(ls.is_some() && ls->type == object_type::lazy_sequence)
    {
      ls = expect_object<lazy_sequence>(ls)->sval();
    }
    return ls;
  }

  lazy_sequence_ref lazy_sequence::with_meta(object_ref const m) const
  {
    auto const ret(make_box<lazy_sequence>(jank_nil, seq()));
    auto const meta(behavior::detail::validate_meta(m));
    ret->meta = meta;
    return ret;
  }
}
