#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::runtime
{
  obj::iterator::static_object(object_ptr const fn, object_ptr const start)
    : fn{ fn }, current{ start }
  { }

  obj::iterator_ptr obj::iterator::seq()
  { return this; }

  obj::iterator_ptr obj::iterator::fresh_seq() const
  { return jank::make_box<obj::iterator>(fn, current); }

  object_ptr obj::iterator::first() const
  { return current; }

  obj::iterator_ptr obj::iterator::next() const
  {
    if(cached_next)
    { return cached_next; }

    auto const next(dynamic_call(fn, current));
    auto const ret(jank::make_box<obj::iterator>(fn, next));
    cached_next = ret;

    return ret;
  }

  obj::iterator_ptr obj::iterator::next_in_place()
  {
    if(cached_next)
    {
      current = cached_next->first();
      cached_next = nullptr;
    }
    else
    {
      auto const next(dynamic_call(fn, current));
      current = next;
    }

    return this;
  }

  object_ptr obj::iterator::next_in_place_first()
  {
    if(cached_next)
    {
      current = cached_next->first();
      cached_next = nullptr;
    }
    else
    {
      auto const next(dynamic_call(fn, current));
      current = next;
    }

    return current;
  }

  native_bool obj::iterator::equal(object const &o) const
  {
    return visit_object
    (
      [this](auto const typed_o)
      {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!behavior::seqable<T>)
        { return false; }
        else
        {
          auto seq(typed_o->fresh_seq());
          for(auto it(fresh_seq()); it != nullptr; seq = seq->next_in_place(), seq = seq->next_in_place())
          {
            if(seq == nullptr || !runtime::detail::equal(it, seq->first()))
            { return false; }
          }
          return true;
        }
      },
      &o
    );
  }

  void obj::iterator::to_string(fmt::memory_buffer &buff)
  { runtime::detail::to_string(seq(), buff); }

  native_persistent_string obj::iterator::to_string()
  { return runtime::detail::to_string(seq()); }

  native_integer obj::iterator::to_hash() const
  {
    size_t hash{};

    for(auto it(fresh_seq()); it != nullptr; it = it->next_in_place())
    { hash = runtime::detail::hash_combine(hash, *it->first()); }

    return static_cast<native_integer>(hash);
  }

  obj::cons_ptr obj::iterator::cons(object_ptr head)
  { return make_box<obj::cons>(head, this); }
}
