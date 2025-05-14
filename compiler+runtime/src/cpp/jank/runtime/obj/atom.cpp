#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  atom::atom(object_ref const o)
    : val{ o.data }
  {
  }

  bool atom::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string atom::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void atom::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
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

  object_ref atom::reset(object_ref const o)
  {
    jank_debug_assert(o.is_some());
    val = o.data;
    return o;
  }

  persistent_vector_ref atom::reset_vals(object_ref const o)
  {
    while(true)
    {
      auto v(val.load());
      if(val.compare_exchange_weak(v, o.data))
      {
        return make_box<persistent_vector>(std::in_place, v, o);
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap) */
  object_ref atom::swap(object_ref const fn)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v));
      if(val.compare_exchange_weak(v, next.data))
      {
        return next;
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap) */
  object_ref atom::swap(object_ref const fn, object_ref const a1)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v, a1));
      if(val.compare_exchange_weak(v, next.data))
      {
        return next;
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap) */
  object_ref atom::swap(object_ref const fn, object_ref const a1, object_ref const a2)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(dynamic_call(fn, v, a1, a2));
      if(val.compare_exchange_weak(v, next.data))
      {
        return next;
      }
    }
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap) */
  object_ref
  atom::swap(object_ref const fn, object_ref const a1, object_ref const a2, object_ref const rest)
  {
    while(true)
    {
      auto v(val.load());
      auto const next(apply_to(fn, conj(a1, conj(a2, rest))));
      if(val.compare_exchange_weak(v, next.data))
      {
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
        return make_box<persistent_vector>(std::in_place, v, next);
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
        return make_box<persistent_vector>(std::in_place, v, next);
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
        return make_box<persistent_vector>(std::in_place, v, next);
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
      auto const next(apply_to(fn, conj(a1, conj(a2, rest))));
      if(val.compare_exchange_weak(v, next.data))
      {
        return make_box<persistent_vector>(std::in_place, v, next);
      }
    }
  }

  object_ref atom::compare_and_set(object_ref const old_val, object_ref const new_val)
  {
    object *old{ old_val.data };
    return make_box(val.compare_exchange_weak(old, new_val.data));
  }
}
