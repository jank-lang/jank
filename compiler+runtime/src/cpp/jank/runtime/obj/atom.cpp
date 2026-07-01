#include <jank/runtime/core.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  atom::atom()
    : object{ obj_type, obj_behaviors }
  {
  }

  atom::atom(object_ref const o)
    : object{ obj_type, obj_behaviors }
    , val{ o.raw() }
    , watches{ persistent_hash_map::empty() }
  {
  }

  object_ref atom::deref() const
  {
    return val.load();
  }

  object_ref atom::with_meta(object_ref const)
  {
    throw std::runtime_error{
      "Value of type 'atom' does not support 'with-meta'.",
    };
  }

  object_ref atom::get_meta() const
  {
    return meta.get();
  }

  void atom::set_meta(object_ref const o)
  {
    auto const new_meta(behavior::detail::validate_meta(o));
    meta.set(new_meta);
  }

  static void notify_watches(atom_ref const a, object_ref const old_val, object_ref const new_val)
  {
    auto const locked_watches(a->watches.rlock());
    for(auto const &entry : (*locked_watches)->data)
    {
      auto const fn(entry.second);
      if(fn.is_some())
      {
        fn.call(entry.first, a, old_val, new_val);
      }
    }
  }

  object_ref atom::reset(object_ref const o)
  {
    validate(o);
    auto const v(val.load());
    val = o.raw();
    notify_watches(runtime::detail::untagged(this), v, o);
    return o;
  }

  persistent_vector_ref atom::reset_vals(object_ref const o)
  {
    validate(o);
    while(true)
    {
      auto v(val.load());
      if(val.compare_exchange_weak(v, o.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, o);
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
      auto const next(fn.call(v));
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
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
      auto const next(fn.call(v, a1));
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
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
      auto const next(fn.call(v, a1, a2));
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
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
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
        return next;
      }
    }
  }

  persistent_vector_ref atom::swap_vals(object_ref const fn)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(fn.call(v));
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
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
      auto const next(fn.call(v, a1));
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
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
      auto const next(fn.call(v, a1, a2));
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
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
      validate(next);
      if(val.compare_exchange_weak(v, next.raw()))
      {
        notify_watches(runtime::detail::untagged(this), v, next);
        auto ret{ make_box<persistent_vector>(std::in_place, v, next) };
        return ret;
      }
    }
  }

  object_ref atom::compare_and_set(object_ref const old_val, object_ref const new_val)
  {
    validate(new_val);
    /* NOLINTNEXTLINE(misc-const-correctness): Can't actually be const. */
    object *old{ old_val.raw() };
    auto const ret(val.compare_exchange_weak(old, new_val.raw()));
    if(ret)
    {
      notify_watches(runtime::detail::untagged(this), old_val, new_val);
    }
    return make_box(ret);
  }

  static void do_validate(object_ref const vf, object_ref const val)
  {
    if(vf.is_some() && !truthy(vf.call(val)))
    {
      throw std::runtime_error{ "Invalid atom state" };
    }
  }

  void atom::set_validator(object_ref const vf)
  {
    do_validate(vf, deref());
    auto locked_validator(validator.wlock());
    *locked_validator = vf;
  }

  object_ref atom::get_validator()
  {
    auto const locked_validator(validator.rlock());
    return *locked_validator;
  }

  void atom::add_watch(object_ref const key, object_ref const fn)
  {
    auto locked_watches(watches.wlock());
    *locked_watches = (*locked_watches)->assoc(key, fn);
  }

  void atom::remove_watch(object_ref const key)
  {
    auto locked_watches(watches.wlock());
    *locked_watches = (*locked_watches)->dissoc(key);
  }

  void atom::validate(object_ref const val)
  {
    auto const locked_validator(validator.rlock());
    do_validate(*locked_validator, val);
  }
}
