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
    util::println("ctor lazy_sequence {} fn {}", this, runtime::to_code_string(fn));
  }

  lazy_sequence::lazy_sequence(object_ref const fn, object_ref const sequence)
    : fn{ fn }
    , s{ sequence }
  {
    jank_debug_assert(fn.is_some());
    util::println("ctor lazy_sequence {} fn {} sequence {}",
                  this,
                  runtime::to_code_string(fn),
                  object_type_str(sequence->type));
  }

  object_ref lazy_sequence::seq() const
  {
    util::println("lazy_sequence {} seq", this);
    realize();
    return s;
  }

  lazy_sequence_ref lazy_sequence::fresh_seq() const
  {
    util::println("lazy_sequence {} fresh_seq", this);
    realize();
    if(s.is_nil())
    {
      return {};
    }
    auto const r(runtime::fresh_seq(s));
    jank_debug_assert(r != nil::nil_const());
    return make_box<lazy_sequence>(nil::nil_const(), r);
  }

  object_ref lazy_sequence::first() const
  {
    util::println("lazy_sequence {} first", this);
    realize();
    if(s.is_nil())
    {
      return nil::nil_const();
    }
    return runtime::first(s);
  }

  lazy_sequence_ref lazy_sequence::next() const
  {
    __builtin_debugtrap();
    util::println("lazy_sequence {} next", this);
    realize();
    if(s.is_nil())
    {
      return {};
    }
    auto const n(runtime::next(s));
    return expect_object<lazy_sequence>(n);
  }

  lazy_sequence_ref lazy_sequence::next_in_place()
  {
    util::println("lazy_sequence {} next_in_place", this);
    realize();
    jank_debug_assert(s.is_some());
    auto const n(runtime::next_in_place(s));
    if(n == nil::nil_const())
    {
      return {};
    }
    s = n;
    return this;
  }

  native_bool lazy_sequence::equal(object const &o) const
  {
    return sequence_equal(this, &o);
  }

  void lazy_sequence::to_string(util::string_builder &buff) const
  {
    util::println("lazy_sequence {} to_string buff", this);
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string lazy_sequence::to_string() const
  {
    util::println("lazy_sequence {} to_string", this);
    return runtime::to_string(seq());
  }

  jtl::immutable_string lazy_sequence::to_code_string() const
  {
    util::println("lazy_sequence {} to_code_string", this);
    return runtime::to_code_string(seq());
  }

  native_hash lazy_sequence::to_hash() const
  {
    util::println("lazy_sequence {} to_hash", this);
    auto const s(seq());
    if(s.is_nil())
    {
      return 1;
    }
    return hash::ordered(s.erase());
  }

  cons_ref lazy_sequence::conj(object_ref const head) const
  {
    util::println("lazy_sequence {} conj", this);
    return make_box<cons>(head, seq());
  }

  //object_ref lazy_sequence::resolve_fn() const
  //{
  //  if(fn.is_some())
  //  {
  //    fn_result = dynamic_call(fn);
  //    fn = nil::nil_const();
  //  }
  //  if(fn_result.is_some())
  //  {
  //    return fn_result;
  //  }
  //  return sequence;
  //}

  //object_ref lazy_sequence::resolve_seq() const
  //{
  //  resolve_fn();
  //  if(fn_result.is_some())
  //  {
  //    object_ref lazy{ fn_result };
  //    fn_result = nil::nil_const();
  //    while(lazy.is_some() && lazy->type == object_type::lazy_sequence)
  //    {
  //      lazy = expect_object<lazy_sequence>(lazy)->resolve_fn();
  //    }
  //    if(lazy.is_some())
  //    {
  //      sequence = runtime::seq(lazy);
  //    }
  //  }
  //  return sequence;
  //}

  void lazy_sequence::realize() const
  {
    /* TODO: Lock. */
    util::println("\tlazy_sequence {} realize", this);
    force();
    auto ls{ sv };
    sv = nil::nil_const();
    util::println("\t\t[realize] {} sv is a {}", this, object_type_str(ls->type));
    if(ls.is_some() && ls->type == object_type::lazy_sequence)
    {
      util::println("\t\t[realize] {} unwrapping ls", this);
      ls = unwrap(ls);
    }
    s = runtime::seq(ls);
    util::println("\t\t[realize] {} s is a {}", this, object_type_str(s->type));
  }

  void lazy_sequence::force() const
  {
    util::println("\tlazy_sequence {} force", this);
    if(fn.is_some())
    {
      util::println("\t\t[force] {} calling fn {}", this, runtime::to_code_string(fn));
      sv = dynamic_call(fn);
      fn = nil::nil_const();
      util::println("\t\t[force] {} done calling, sv is a {}", this, object_type_str(sv->type));
    }
    else
    {
      util::println("\t\t[force] {} fn is nil", this);
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
    auto const ret(make_box<lazy_sequence>(nil::nil_const(), seq()));
    auto const meta(behavior::detail::validate_meta(m));
    ret->meta = meta;
    return ret;
  }
}
