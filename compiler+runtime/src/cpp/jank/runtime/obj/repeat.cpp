#include <jank/runtime/obj/repeat.hpp>

namespace jank::runtime
{

  obj::repeat::static_object(object_ptr const value)
    : value{ value }
    , count{ make_box(infinite) }
  {
  }

  obj::repeat::static_object(object_ptr const count, object_ptr const value)
    : value{ value }
    , count{ count }
  {
  }

  object_ptr obj::repeat::create(object_ptr const value)
  {
    return make_box<obj::repeat>(value);
  }

  object_ptr obj::repeat::create(object_ptr const count, object_ptr const value)
  {
    if(lte(count, make_box(0)))
    {
      return obj::persistent_list::empty();
    }
    return make_box<obj::repeat>(count, value);
  }

  obj::repeat_ptr obj::repeat::seq()
  {
    return this;
  }

  obj::repeat_ptr obj::repeat::fresh_seq() const
  {
    return make_box<obj::repeat>(count, value);
  }

  object_ptr obj::repeat::first() const
  {
    return value;
  }

  obj::repeat_ptr obj::repeat::next() const
  {
    if(runtime::equal(count, make_box(infinite)))
    {
      return this;
    }
    if(lt(count, make_box(1)))
    {
      return nullptr;
    }
    return make_box<obj::repeat>(make_box(add(count, make_box(-1))), value);
  }

  obj::repeat_ptr obj::repeat::next_in_place()
  {
    if(runtime::equal(count, make_box(infinite)))
    {
      return this;
    }
    if(lte(count, make_box(1)))
    {
      return nullptr;
    }
    count = add(count, make_box(-1));
    return this;
  }

  obj::cons_ptr obj::repeat::conj(object_ptr const head) const
  {
    return make_box<obj::cons>(head, this);
  }

  native_bool obj::repeat::equal(object const &o) const
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
          /* TODO: This is common code; can it be shared? */
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

  void obj::repeat::to_string(fmt::memory_buffer &buff)
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string obj::repeat::to_string()
  {
    return runtime::to_string(seq());
  }

  native_persistent_string obj::repeat::to_code_string()
  {
    return runtime::to_code_string(seq());
  }

  native_hash obj::repeat::to_hash() const
  {
    return hash::ordered(&base);
  }

  obj::repeat_ptr obj::repeat::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
