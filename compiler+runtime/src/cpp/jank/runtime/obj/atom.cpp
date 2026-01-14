#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  atom::atom(object_ref const o)
    : val{ o.data }
    , watches{ persistent_hash_map::empty() }
  {
  }

  bool atom::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string atom::to_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void atom::to_string(jtl::string_builder &buff) const
  {
    util::format_to(buff, "#object [{} {}]", object_type_str(base.type), &base);
  }

  jtl::immutable_string atom::to_code_string() const
  {
    return to_string();
  }

  uhash atom::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ref atom::deref() const
  {
    return val.load();
  }

  static void notify_watches(atom_ref const a, object_ref const old_val, object_ref const new_val)
  {
    auto const locked_watches(a->watches.rlock());
    for(auto const &entry : (*locked_watches)->data)
    {
      auto const fn(entry.second);
      if(fn.is_some())
      {
        dynamic_call(fn, entry.first, a, old_val, new_val);
      }
    }
  }

  object_ref atom::reset(object_ref const o)
  {
    jank_debug_assert(o.is_some());
    auto const v(val.load());
    val = o.data;
    notify_watches(this, v, o);
    return o;
  }

  persistent_vector_ref atom::reset_vals(object_ref const o)
  {
    while(true)
    {
      auto v(val.load());
      if(val.compare_exchange_weak(v, o.data))
      {
        notify_watches(this, v, o);
        auto ret{ make_box<persistent_vector>(std::in_place, v, o) };
        return ret;
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,bugprone-exception-escape) */
  object_ref atom::swap(object_ref const fn)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        return next;
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,bugprone-exception-escape) */
  object_ref atom::swap(object_ref const fn, object_ref const a1)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v, a1));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        return next;
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,bugprone-exception-escape) */
  object_ref atom::swap(object_ref const fn, object_ref const a1, object_ref const a2)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v, a1, a2));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        return next;
      }
    }
  }

  object_ref
  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,bugprone-exception-escape) */
  atom::swap(object_ref const fn, object_ref const a1, object_ref const a2, object_ref const rest)
  {
    while(true)
    {
      auto v(val.load());
      auto const args(runtime::cons(v, runtime::cons(a1, runtime::cons(a2, rest))));
      auto const next(apply_to(fn, args));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        return next;
      }
    }
  }

  persistent_vector_ref atom::swap_vals(object_ref const fn)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        auto ret{ make_box<persistent_vector>(std::in_place, v, next) };
        return ret;
      }
    }
  }

  persistent_vector_ref atom::swap_vals(object_ref const fn, object_ref const a1)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v, a1));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        auto ret{ make_box<persistent_vector>(std::in_place, v, next) };
        return ret;
      }
    }
  }

  persistent_vector_ref
  atom::swap_vals(object_ref const fn, object_ref const a1, object_ref const a2)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v, a1, a2));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        auto ret{ make_box<persistent_vector>(std::in_place, v, next) };
        return ret;
      }
    }
  }

  persistent_vector_ref atom::swap_vals(object_ref const fn,
                                        object_ref const a1,
                                        object_ref const a2,
                                        object_ref const rest)
  {
    while(true)
    {
      auto v(val.load());
      auto const args(runtime::cons(v, runtime::cons(a1, runtime::cons(a2, rest))));
      auto const next(apply_to(fn, args));
      if(val.compare_exchange_weak(v, next.data))
      {
        notify_watches(this, v, next);
        auto ret{ make_box<persistent_vector>(std::in_place, v, next) };
        return ret;
      }
    }
  }

  object_ref atom::compare_and_set(object_ref const old_val, object_ref const new_val)
  {
    /* NOLINTNEXTLINE(misc-const-correctness): Can't actually be const. */
    object *old{ old_val.data };
    auto const ret(val.compare_exchange_weak(old, new_val.data));
    if(ret)
    {
      notify_watches(this, old_val, new_val);
    }
    return make_box(ret);
  }

  void atom::add_watch(object_ref const key, object_ref const fn)
  {
    auto locked_watches(this->watches.wlock());
    *locked_watches = (*locked_watches)->assoc(key, fn);
  }

  void atom::remove_watch(object_ref const key)
  {
    auto locked_watches(this->watches.wlock());
    *locked_watches = (*locked_watches)->dissoc(key);
  }
}
