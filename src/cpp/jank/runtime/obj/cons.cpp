#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::runtime
{
  obj::cons::static_object(object_ptr const head, object_ptr const tail)
    : head{ head }
    , tail{ tail }
  {
    assert(head);
  }

  obj::cons_ptr obj::cons::seq()
  {
    return this;
  }

  obj::cons_ptr obj::cons::fresh_seq() const
  {
    return make_box<obj::cons>(head, tail);
  }

  object_ptr obj::cons::first() const
  {
    return head;
  }

  obj::cons_ptr obj::cons::next() const
  {
    if(!tail)
    {
      return nullptr;
    }

    return visit_object(
      [&](auto const typed_tail) -> obj::cons_ptr {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          return make_box<obj::cons>(typed_tail->first(), typed_tail->next());
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_tail->to_string()) };
        }
      },
      tail);
  }

  obj::cons_ptr obj::cons::next_in_place()
  {
    if(!tail)
    {
      return nullptr;
    }

    visit_object(
      [&](auto const typed_tail) {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          head = typed_tail->first();
          tail = typed_tail->next();
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_tail->to_string()) };
        }
      },
      tail);

    return this;
  }

  object_ptr obj::cons::next_in_place_first()
  {
    if(!tail)
    {
      return nullptr;
    }

    visit_object(
      [&](auto const typed_tail) {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          head = typed_tail->first();
          tail = typed_tail->next();
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_tail->to_string()) };
        }
      },
      tail);

    return head;
  }

  native_bool obj::cons::equal(object const &o) const
  {
    return visit_object(
      [this](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!behavior::seqable<T>)
        {
          return false;
        }
        else
        {
          auto seq(typed_o->fresh_seq());
          for(auto it(fresh_seq()); it != nullptr;
              seq = seq->next_in_place(), seq = seq->next_in_place())
          {
            if(seq == nullptr || !runtime::detail::equal(it, seq->first()))
            {
              return false;
            }
          }
          return true;
        }
      },
      &o);
  }

  void obj::cons::to_string(fmt::memory_buffer &buff)
  {
    runtime::detail::to_string(seq(), buff);
  }

  native_persistent_string obj::cons::to_string()
  {
    return runtime::detail::to_string(seq());
  }

  native_hash obj::cons::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::ordered(&base);
  }

  obj::cons_ptr obj::cons::cons(object_ptr head) const
  {
    return make_box<obj::cons>(head, this);
  }
}
