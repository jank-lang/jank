#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime
{
  obj::cons::static_object(object_ptr const head, object_ptr const tail)
    : head{ head }
    , tail{ tail == obj::nil::nil_const() ? nullptr : tail }
  {
    assert(head);
  }

  obj::cons_ptr obj::cons::seq() const
  {
    return const_cast<obj::cons *>(this);
  }

  obj::cons_ptr obj::cons::fresh_seq() const
  {
    return make_box<obj::cons>(head, tail);
  }

  object_ptr obj::cons::first() const
  {
    return head;
  }

  object_ptr obj::cons::next() const
  {
    if(!tail)
    {
      return nullptr;
    }

    return runtime::seq(tail);
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
          if(tail == obj::nil::nil_const())
          {
            tail = nullptr;
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_tail->to_string()) };
        }
      },
      tail);

    return this;
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
              it = runtime::next_in_place(it), seq = runtime::next_in_place(seq))
          {
            if(seq == nullptr || !runtime::equal(it, seq->first()))
            {
              return false;
            }
          }
          return true;
        }
      },
      &o);
  }

  void obj::cons::to_string(fmt::memory_buffer &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string obj::cons::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string obj::cons::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::cons::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::ordered(&base);
  }

  obj::cons_ptr obj::cons::conj(object_ptr const head) const
  {
    return make_box<obj::cons>(head, this);
  }

  obj::cons_ptr obj::cons::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
