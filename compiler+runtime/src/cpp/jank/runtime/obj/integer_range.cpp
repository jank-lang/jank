#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  static native_bool positive_step_bounds_check(integer_ptr const val, integer_ptr const end)
  {
    return lte(end, val);
  }

  static native_bool negative_step_bounds_check(integer_ptr const val, integer_ptr const end)
  {
    return lte(val, end);
  }

  integer_range::integer_range(integer_ptr const end)
    : start{ make_box<integer>(0) }
    , end{ end }
    , step{ make_box<integer>(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  integer_range::integer_range(integer_ptr const start, integer_ptr const end)
    : start{ start }
    , end{ end }
    , step{ make_box<integer>(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  integer_range::integer_range(integer_ptr const start,
                               integer_ptr const end,
                               integer_ptr const step)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ lt(static_cast<native_integer>(0), step) ? positive_step_bounds_check
                                                             : negative_step_bounds_check }
  {
  }

  integer_range::integer_range(integer_ptr const start,
                               integer_ptr const end,
                               integer_ptr const step,
                               integer_range::bounds_check_t const bounds_check)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  object_ptr integer_range::create(integer_ptr const end)
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

  object_ptr integer_range::create(integer_ptr const start, integer_ptr const end)
  {
    return create(start, end, make_box<integer>(1));
  }

  object_ptr
  integer_range::create(integer_ptr const start, integer_ptr const end, integer_ptr const step)
  {
    if((is_pos(step) && lt(end, start)) || (is_neg(step) && lt(start, end)) || is_equiv(start, end))
    {
      return persistent_list::empty();
    }
    else if(is_zero(step))
    {
      return repeat::create(make_box<integer>(start));
    }
    return make_box<integer_range>(start,
                                   end,
                                   step,
                                   is_pos(step) ? positive_step_bounds_check
                                                : negative_step_bounds_check);
  }

  integer_range_ptr integer_range::seq() const
  {
    return this;
  }

  integer_range_ptr integer_range::fresh_seq() const
  {
    return make_box<integer_range>(start, end, step, bounds_check);
  }

  integer_ptr integer_range::first() const
  {
    return start;
  }

  integer_range_ptr integer_range::next() const
  {
    if(count() <= 1)
    {
      return nullptr;
    }
    return make_box<integer_range>(make_box<integer>(add(start, step)), end, step, bounds_check);
  }

  integer_range_ptr integer_range::next_in_place()
  {
    if(count() <= 1)
    {
      return nullptr;
    }
    start = make_box<integer>(add(start, step));
    return this;
  }

  cons_ptr integer_range::conj(object_ptr const head) const
  {
    return make_box<cons>(head, this);
  }

  native_bool integer_range::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        /* TODO: This is common code; can it be shared? */
        for(auto it(fresh_seq()); it != nullptr;
            it = it->next_in_place(), seq = seq->next_in_place())
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

  void integer_range::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string integer_range::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string integer_range::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash integer_range::to_hash() const
  {
    return hash::ordered(&base);
  }

  integer_range_ptr integer_range::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto const ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

  size_t integer_range::count() const
  {
    auto const s{ step->data };
    if(s == 0)
    {
      throw std::runtime_error("[Integer-Range] Step shouldn't be 0");
    }

    auto const diff{ sub(end, start) };
    auto const offset{ s > 0 ? -1 : 1 };

    if((s > 0 && diff > std::numeric_limits<native_integer>::max() - s + offset)
       || (s < 0 && diff < std::numeric_limits<native_integer>::min() - s + offset))
    {
      throw std::runtime_error("[Integer-Range] Overflow occurred in arithmetic");
    }

    return static_cast<size_t>((diff + offset + s) / s);
  }
}
