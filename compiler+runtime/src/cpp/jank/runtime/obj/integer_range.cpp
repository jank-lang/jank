#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  static native_bool positive_step_bounds_check(integer_ref const val, integer_ref const end)
  {
    return lte(end, val);
  }

  static native_bool negative_step_bounds_check(integer_ref const val, integer_ref const end)
  {
    return lte(val, end);
  }

  integer_range::integer_range(integer_ref const end)
    : start{ make_box<integer>(0) }
    , end{ end }
    , step{ make_box<integer>(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  integer_range::integer_range(integer_ref const start, integer_ref const end)
    : start{ start }
    , end{ end }
    , step{ make_box<integer>(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  integer_range::integer_range(integer_ref const start,
                               integer_ref const end,
                               integer_ref const step)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ lt(static_cast<i64>(0), step.erase()) ? positive_step_bounds_check
                                                                     : negative_step_bounds_check }
  {
  }

  integer_range::integer_range(integer_ref const start,
                               integer_ref const end,
                               integer_ref const step,
                               integer_range::bounds_check_t const bounds_check)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  object_ref integer_range::create(integer_ref const end)
  {
    if(is_pos(end))
    {
      return make_box<integer_range>(make_box<integer>(0),
                                     end,
                                     make_box<integer>(1),
                                     positive_step_bounds_check);
    }
    return persistent_list::empty();
  }

  object_ref integer_range::create(integer_ref const start, integer_ref const end)
  {
    return create(start, end, make_box<integer>(1));
  }

  object_ref
  integer_range::create(integer_ref const start, integer_ref const end, integer_ref const step)
  {
    if((is_pos(step) && lt(end, start)) || (is_neg(step) && lt(start, end)) || is_equiv(start, end))
    {
      return persistent_list::empty();
    }
    else if(is_zero(step))
    {
      return repeat::create(start);
    }
    return make_box<integer_range>(start,
                                   end,
                                   step,
                                   is_pos(step) ? positive_step_bounds_check
                                                : negative_step_bounds_check);
  }

  integer_range_ref integer_range::seq() const
  {
    return this;
  }

  integer_range_ref integer_range::fresh_seq() const
  {
    return make_box<integer_range>(start, end, step, bounds_check);
  }

  integer_ref integer_range::first() const
  {
    return start;
  }

  integer_range_ref integer_range::next() const
  {
    if(count() <= 1)
    {
      return {};
    }
    return make_box<integer_range>(make_box<integer>(add(start, step)), end, step, bounds_check);
  }

  integer_range_ref integer_range::next_in_place()
  {
    if(count() <= 1)
    {
      return {};
    }
    start = make_box<integer>(add(start, step));
    return this;
  }

  cons_ref integer_range::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }

  native_bool integer_range::equal(object const &o) const
  {
    return runtime::sequence_equal(this, &o);
  }

  void integer_range::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string integer_range::to_string() const
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string integer_range::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash integer_range::to_hash() const
  {
    return hash::ordered(&base);
  }

  integer_range_ref integer_range::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto const ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

  usize integer_range::count() const
  {
    auto const s{ step->data };
    if(s == 0)
    {
      throw std::runtime_error("[Integer-Range] Step shouldn't be 0");
    }

    auto const diff{ sub(end, start) };
    auto const offset{ s > 0 ? -1 : 1 };

    if((s > 0 && diff > std::numeric_limits<i64>::max() - s + offset)
       || (s < 0 && diff < std::numeric_limits<i64>::min() - s + offset))
    {
      throw std::runtime_error("[Integer-Range] Overflow occurred in arithmetic");
    }

    return static_cast<size_t>((diff + offset + s) / s);
  }
}
