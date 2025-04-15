#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  cons::cons(object_ref const head, object_ref const tail)
    : head{ head }
    , tail{ tail }
  {
  }

  cons_ref cons::seq() const
  {
    return this;
  }

  cons_ref cons::fresh_seq() const
  {
    return make_box<cons>(head, tail);
  }

  object_ref cons::first() const
  {
    return head;
  }

  object_ref cons::next() const
  {
    if(!tail)
    {
      return {};
    }

    return runtime::seq(tail);
  }

  cons_ref cons::next_in_place()
  {
    if(!tail)
    {
      return {};
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
            tail = nil::nil_const();
          }
        }
        else
        {
          throw std::runtime_error{ util::format("invalid sequence: {}", typed_tail->to_string()) };
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
        for(auto it(fresh_seq()); it; it = it->next_in_place(), seq = seq->next_in_place())
        {
          if(!seq || !runtime::equal(it->first(), seq->first()))
          {
            return false;
          }
        }
        return !seq;
      },
      []() { return false; },
      &o);
  }

  void cons::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string cons::to_string() const
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string cons::to_code_string() const
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

  cons_ref cons::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }

  cons_ref cons::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
