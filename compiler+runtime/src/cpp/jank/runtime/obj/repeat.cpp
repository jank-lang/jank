#include <jank/runtime/obj/repeat.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  repeat::repeat(object_ref const value)
    : value{ value }
    , count{ make_box(infinite) }
  {
  }

  repeat::repeat(object_ref const count, object_ref const value)
    : value{ value }
    , count{ count }
  {
    if(0 >= to_int(count))
    {
      throw std::runtime_error{ "repeat must be constructed with positive count: "
                                + std::to_string(to_int(count)) };
    }
  }

  object_ref repeat::create(object_ref const value)
  {
    return make_box<repeat>(value);
  }

  object_ref repeat::create(object_ref const count, object_ref const value)
  {
    if(lte(count, make_box(0)))
    {
      return persistent_list::empty();
    }
    return make_box<repeat>(count, value);
  }

  repeat_ref repeat::seq()
  {
    return this;
  }

  repeat_ref repeat::fresh_seq() const
  {
    if(runtime::equal(count, make_box(infinite)))
    {
      return this;
    }
    return make_box<repeat>(count, value);
  }

  object_ref repeat::first() const
  {
    return value;
  }

  repeat_ref repeat::next() const
  {
    if(runtime::equal(count, make_box(infinite)))
    {
      return this;
    }
    if(lte(count, make_box(1)))
    {
      return {};
    }
    return make_box<repeat>(make_box(add(count, make_box(-1))), value);
  }

  repeat_ref repeat::next_in_place()
  {
    if(runtime::equal(count, make_box(infinite)))
    {
      return this;
    }
    if(lte(count, make_box(1)))
    {
      return {};
    }
    count = add(count, make_box(-1));
    return this;
  }

  cons_ref repeat::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }

  native_bool repeat::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        /* TODO: This is common code; can it be shared? */
        for(auto it(fresh_seq()); it.is_some();
            it = it->next_in_place(), seq = seq->next_in_place())
        {
          if(seq.is_nil() || !runtime::equal(it->first(), seq->first()))
          {
            return false;
          }
        }
        return seq.is_nil();
      },
      []() { return false; },
      &o);
  }

  void repeat::to_string(util::string_builder &buff)
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string repeat::to_string()
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string repeat::to_code_string()
  {
    return runtime::to_code_string(seq());
  }

  native_hash repeat::to_hash() const
  {
    return hash::ordered(&base);
  }

  repeat_ref repeat::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
