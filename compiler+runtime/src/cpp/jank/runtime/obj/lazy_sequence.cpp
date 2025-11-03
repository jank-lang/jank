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
  lazy_sequence::lazy_sequence(lazy_sequence const &other)
  {
    base = other.base;
    meta = other.meta;
    auto other_lock(other.lock_state());
    auto lock(lock_state());
    *lock = *other_lock;
  }

  lazy_sequence &lazy_sequence::operator=(lazy_sequence const &other)
  {
    if(this == &other)
    {
      return *this;
    }

    base = other.base;
    meta = other.meta;
    auto other_lock(other.lock_state());
    auto lock(lock_state());
    *lock = *other_lock;
    return *this;
  }

  lazy_sequence::lazy_sequence(object_ref const fn)
  {
    jank_debug_assert(fn.is_some());
    auto lock(lock_state());
    lock->fn = fn;
  }

  lazy_sequence::lazy_sequence(object_ref const fn, object_ref const sequence)
  {
    auto lock(lock_state());
    lock->fn = fn;
    lock->s = sequence;
  }

  folly::Synchronized<lazy_sequence::state, std::recursive_mutex>::LockedPtr
  lazy_sequence::lock_state() const
  {
    return const_cast<folly::Synchronized<state, std::recursive_mutex> &>(state_).lock();
  }

  void lazy_sequence::force_locked(
    folly::Synchronized<state, std::recursive_mutex>::LockedPtr &lock) const
  {
    if(lock->fn.is_nil())
    {
      return;
    }

    lock->sv = dynamic_call(lock->fn);
    lock->fn = jank_nil;
  }

  object_ref lazy_sequence::seq() const
  {
    realize();
    auto lock(lock_state());
    return lock->s;
  }

  lazy_sequence_ref lazy_sequence::fresh_seq() const
  {
    auto const sequence(seq());
    if(sequence.is_nil())
    {
      return {};
    }
    auto const r(runtime::fresh_seq(sequence));
    jank_debug_assert(r != jank_nil);
    return make_box<lazy_sequence>(jank_nil, r);
  }

  object_ref lazy_sequence::first() const
  {
    auto const sequence(seq());
    if(sequence.is_nil())
    {
      return sequence;
    }
    return runtime::first(sequence);
  }

  object_ref lazy_sequence::next() const
  {
    auto const sequence(seq());
    if(sequence.is_nil())
    {
      return {};
    }
    auto const n(runtime::next(sequence));
    return n;
  }

  bool lazy_sequence::equal(object const &o) const
  {
    return sequence_equal(this, &o);
  }

  void lazy_sequence::to_string(jtl::string_builder &buff) const
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
    auto lock(lock_state());
    force_locked(lock);
    if(lock->sv.is_nil())
    {
      return;
    }

    auto ls(lock->sv);
    lock->sv = jank_nil;
    if(ls.is_some() && ls->type == object_type::lazy_sequence)
    {
      ls = unwrap(ls);
    }
    lock->s = runtime::seq(ls);
  }

  void lazy_sequence::force() const
  {
    auto lock(lock_state());
    force_locked(lock);
  }

  void lazy_sequence::lock_and_force() const
  {
    auto lock(lock_state());
    force_locked(lock);
  }

  object_ref lazy_sequence::sval() const
  {
    auto lock(lock_state());
    force_locked(lock);
    if(lock->sv.is_some())
    {
      return lock->sv;
    }
    return lock->s;
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
