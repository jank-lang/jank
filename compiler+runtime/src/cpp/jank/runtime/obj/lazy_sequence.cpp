#include <jank/runtime/obj/lazy_sequence.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  lazy_sequence::lazy_sequence()
    : object{ obj_type, obj_behaviors }
  {
  }

  lazy_sequence::lazy_sequence(object_ref const fn)
    : object{ obj_type, obj_behaviors }
    , fn{ fn }
  {
    jank_debug_assert(fn.is_some());
  }

  lazy_sequence::lazy_sequence(object_ref const fn, object_ref const sequence)
    : object{ obj_type, obj_behaviors }
    , fn{ fn }
    , s{ sequence }
  {
  }

  object_ref lazy_sequence::seq() const
  {
    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    return realize();
  }

  lazy_sequence_ref lazy_sequence::fresh_seq() const
  {
    auto const ret{ realize() };
    if(ret.is_nil())
    {
      return {};
    }

    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    auto const r(runtime::fresh_seq(s));
    jank_debug_assert(r != jank_nil());
    return make_box<lazy_sequence>(jank_nil(), r);
  }

  object_ref lazy_sequence::first() const
  {
    auto const ret{ realize() };
    if(ret.is_nil())
    {
      return ret;
    }

    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    return runtime::first(s);
  }

  object_ref lazy_sequence::next() const
  {
    auto const ret{ realize() };
    if(ret.is_nil())
    {
      return {};
    }

    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    auto const n(runtime::next(s));
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
    return hash::ordered(s.erase().data);
  }

  cons_ref lazy_sequence::conj(object_ref const head) const
  {
    return make_box<cons>(head, seq());
  }

  object_ref lazy_sequence::realize() const
  {
    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    force();
    if(sv.is_some())
    {
      auto ls{ sv };
      sv = jank_nil();
      if(ls.is_some() && ls->type == object_type::lazy_sequence)
      {
        ls = unwrap(ls);
      }
      s = runtime::seq(ls);
    }
    return s;
  }

  /* XXX: Must be locked when called. */
  void lazy_sequence::force() const
  {
    if(fn.is_some())
    {
      sv = dynamic_call(fn);
      fn = jank_nil();
    }
  }

  object_ref lazy_sequence::sval() const
  {
    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    if(fn.is_some())
    {
      force();
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

  bool lazy_sequence::is_realized() const
  {
    std::lock_guard<std::recursive_mutex> const lock{ mutex };
    return fn.is_some() || sv.is_some();
  }

  lazy_sequence_ref lazy_sequence::with_meta(object_ref const m) const
  {
    auto const ret(make_box<lazy_sequence>(jank_nil(), seq()));
    auto const meta(behavior::detail::validate_meta(m));
    ret->meta = meta;
    return ret;
  }

  object_ref lazy_sequence::get_meta() const
  {
    return meta;
  }
}
