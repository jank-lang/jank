#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  static bool positive_step_bounds_check(i64 const val, i64 const end)
  {
    return end <= val;
  }

  static bool negative_step_bounds_check(i64 const val, i64 const end)
  {
    return val <= end;
  }

  integer_range::integer_range()
    : object{ obj_type, obj_behaviors }
  {
  }

  integer_range::integer_range(i64 const end)
    : object{ obj_type, obj_behaviors }
    , end{ end }
    , bounds_check{ static_cast<bounds_check_t>(positive_step_bounds_check) }
  {
  }

  integer_range::integer_range(i64 const start, i64 const end)
    : object{ obj_type, obj_behaviors }
    , start{ start }
    , end{ end }
    , bounds_check{ static_cast<bounds_check_t>(positive_step_bounds_check) }
  {
  }

  integer_range::integer_range(i64 const start, i64 const end, i64 const step)
    : object{ obj_type, obj_behaviors }
    , start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ 0 < step ? static_cast<bounds_check_t>(positive_step_bounds_check)
                             : static_cast<bounds_check_t>(negative_step_bounds_check) }
  {
  }

  integer_range::integer_range(i64 const start,
                               i64 const end,
                               i64 const step,
                               integer_range::bounds_check_t const bounds_check)
    : object{ obj_type, obj_behaviors }
    , start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  object_ref integer_range::create(i64 const end)
  {
    if(0 < end)
    {
      return make_box<integer_range>(0,
                                     end,
                                     1,
                                     static_cast<bounds_check_t>(positive_step_bounds_check));
    }
    return persistent_list::empty();
  }

  object_ref integer_range::create(i64 const start, i64 const end)
  {
    return create(start, end, 1);
  }

  object_ref integer_range::create(i64 const start, i64 const end, i64 const step)
  {
    if(((0 < step) && (end < start)) || ((step < 0) && (start < end)) || (start == end))
    {
      return persistent_list::empty();
    }

    else if(step == 0)
    {
      return repeat::create(make_box(start));
    }
    return make_box<integer_range>(start,
                                   end,
                                   step,
                                   0 < step
                                     ? static_cast<bounds_check_t>(positive_step_bounds_check)
                                     : static_cast<bounds_check_t>(negative_step_bounds_check));
  }

  integer_range_ref integer_range::seq() const
  {
    return this;
  }

  integer_range_ref integer_range::fresh_seq() const
  {
    return make_box<integer_range>(start, end, step, bounds_check);
  }

  object_ref integer_range::first() const
  {
    return make_box(start);
  }

  integer_range_ref integer_range::next() const
  {
    if(count() <= 1)
    {
      return {};
    }
    return make_box<integer_range>(start + step, end, step, bounds_check);
  }

  integer_range_ref integer_range::next_in_place()
  {
    if(count() <= 1)
    {
      return {};
    }
    start += step;
    return this;
  }

  cons_ref integer_range::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }

  bool integer_range::equal(object const &o) const
  {
    return runtime::sequence_equal(this, &o);
  }

  void integer_range::to_string(jtl::string_builder &buff) const
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

  uhash integer_range::to_hash() const
  {
    return hash::ordered(this);
  }

  integer_range_ref integer_range::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto const ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

  object_ref integer_range::get_meta() const
  {
    return meta;
  }

  usize integer_range::count() const
  {
    auto const s{ step };
    if(s == 0)
    {
      throw std::runtime_error("integer_range step shouldn't be 0.");
    }

    auto const diff{ end - start };
    auto const offset{ s > 0 ? -1 : 1 };

    if((s > 0 && diff > std::numeric_limits<i64>::max() - s + offset)
       || (s < 0 && diff < std::numeric_limits<i64>::min() - s + offset))
    {
      throw std::runtime_error("integer_range overflow occurred in arithmetic.");
    }

    return static_cast<size_t>((diff + offset + s) / s);
  }
}
