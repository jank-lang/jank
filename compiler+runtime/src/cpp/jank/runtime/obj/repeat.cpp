#include <jank/runtime/obj/repeat.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  repeat::repeat()
    : object{ obj_type, obj_behaviors }
  {
  }

  repeat::repeat(object_ref const value)
    : object{ obj_type, obj_behaviors }
    , value{ value }
    , count{ infinite }
  {
  }

  repeat::repeat(i64 const count, object_ref const value)
    : object{ obj_type, obj_behaviors }
    , value{ value }
    , count{ count }
  {
    if(0 >= count)
    {
      throw std::runtime_error{ "repeat must be constructed with positive count: "
                                + std::to_string(count) };
    }
  }

  object_ref repeat::create(object_ref const value)
  {
    return make_box<repeat>(value);
  }

  object_ref repeat::create(i64 const count, object_ref const value)
  {
    if(count <= 0)
    {
      return persistent_list::empty();
    }
    return make_box<repeat>(count, value);
  }

  repeat_ref repeat::seq() const
  {
    return runtime::detail::untagged(this);
  }

  repeat_ref repeat::fresh_seq() const
  {
    if(count == infinite)
    {
      return runtime::detail::untagged(this);
    }
    return make_box<repeat>(count, value);
  }

  object_ref repeat::first() const
  {
    return value;
  }

  repeat_ref repeat::next() const
  {
    if(count == infinite)
    {
      return runtime::detail::untagged(this);
    }
    if(count <= 1)
    {
      return {};
    }
    return make_box<repeat>(count - 1, value);
  }

  repeat_ref repeat::next_in_place()
  {
    if(count == infinite)
    {
      return runtime::detail::untagged(this);
    }
    if(count <= 1)
    {
      return {};
    }
    count = count - 1;
    return runtime::detail::untagged(this);
  }

  cons_ref repeat::conj(object_ref const head) const
  {
    return make_box<cons>(head, runtime::detail::untagged(this));
  }

  bool repeat::equal(object const &o) const
  {
    return runtime::sequence_equal(runtime::detail::untagged(this), runtime::detail::untagged(&o));
  }

  void repeat::to_string(jtl::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string repeat::to_string() const
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string repeat::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  uhash repeat::to_hash() const
  {
    return hash::ordered(runtime::detail::untagged(this));
  }

  repeat_ref repeat::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

  object_ref repeat::get_meta() const
  {
    return meta.get();
  }

  void repeat::set_meta(object_ref const o)
  {
    auto const new_meta(behavior::detail::validate_meta(o));
    meta.set(new_meta);
  }
}
