#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  cons::cons(object_ptr const head, object_ptr const tail)
    : head{ head }
    , tail{ tail == nil::nil_const() ? nullptr : tail }
  {
    assert(head);
  }

  cons_ptr cons::seq() const
  {
    return const_cast<cons *>(this);
  }

  cons_ptr cons::fresh_seq() const
  {
    return make_box<cons>(head, tail);
  }

  object_ptr cons::first() const
  {
    return head;
  }

  object_ptr cons::next() const
  {
    if(!tail)
    {
      return nullptr;
    }

    return runtime::seq(tail);
  }

  cons_ptr cons::next_in_place()
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
          if(tail == nil::nil_const())
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

  native_bool cons::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        for(auto it(fresh_seq()); it != nullptr;
            it = runtime::next_in_place(it), seq = runtime::next_in_place(seq))
        {
          if(seq == nullptr || !runtime::equal(it->first(), seq->first()))
          {
            return false;
          }
        }
        return seq == nullptr;
      },
      []() { return false; },
      &o);
  }

  void cons::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string cons::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string cons::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash cons::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::ordered(&base);
  }

  cons_ptr cons::conj(object_ptr const head) const
  {
    return make_box<cons>(head, this);
  }

  cons_ptr cons::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
