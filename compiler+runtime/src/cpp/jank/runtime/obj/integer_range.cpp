#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  static native_bool
  positive_step_bounds_check(obj::integer_ptr const val, obj::integer_ptr const end)
  {
    return lte(end, val);
  }

  static native_bool
  negative_step_bounds_check(obj::integer_ptr const val, obj::integer_ptr const end)
  {
    return lte(val, end);
  }

  obj::integer_range::static_object(obj::integer_ptr const end)
    : end{ end }
    , step{ make_box<obj::integer>(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  obj::integer_range::static_object(obj::integer_ptr const start, obj::integer_ptr const end)
    : start{ start }
    , end{ end }
    , step{ make_box<obj::integer>(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  obj::integer_range::static_object(obj::integer_ptr const start,
                                    obj::integer_ptr const end,
                                    obj::integer_ptr const step)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ lt(static_cast<native_integer>(0), step) ? positive_step_bounds_check
                                                             : negative_step_bounds_check }
  {
  }

  obj::integer_range::static_object(obj::integer_ptr const start,
                                    obj::integer_ptr const end,
                                    obj::integer_ptr const step,
                                    obj::integer_range::bounds_check_t const bounds_check)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  object_ptr obj::integer_range::create(obj::integer_ptr const end)
  {
    if(is_pos(end))
    {
      return make_box<obj::integer_range>(make_box<obj::integer>(0),
                                          end,
                                          make_box<obj::integer>(1),
                                          positive_step_bounds_check);
    }
    return obj::persistent_list::empty();
  }

  object_ptr obj::integer_range::create(obj::integer_ptr const start, obj::integer_ptr const end)
  {
    return create(start, end, make_box<obj::integer_ptr>(1));
  }

  object_ptr obj::integer_range::create(obj::integer_ptr const start,
                                        obj::integer_ptr const end,
                                        obj::integer_ptr const step)
  {
    if((is_pos(step) && lt(end, start)) || (is_neg(step) && lt(start, end)) || is_equiv(start, end))
    {
      return obj::persistent_list::empty();
    }
    else if(is_zero(step))
    {
      return make_box<obj::repeat>(make_box<obj::integer>(start));
    }
    return make_box<obj::integer_range>(start,
                                        end,
                                        step,
                                        is_pos(step) ? positive_step_bounds_check
                                                     : negative_step_bounds_check);
  }

  obj::integer_range_ptr obj::integer_range::seq() const
  {
    return this;
  }

  obj::integer_range_ptr obj::integer_range::fresh_seq() const
  {
    return make_box<obj::integer_range>(start, end, step, bounds_check);
  }

  obj::integer_ptr obj::integer_range::first() const
  {
    return make_box<obj::integer>(start);
  }

  obj::integer_range_ptr obj::integer_range::next() const
  {
    if(count() <= 1)
    {
      return nullptr;
    }
    return make_box<obj::integer_range>(make_box<obj::integer>(add(start, step)),
                                        end,
                                        step,
                                        bounds_check);
  }

  obj::integer_range_ptr obj::integer_range::next_in_place()
  {
    if(count() <= 1)
    {
      return nullptr;
    }
    start = make_box<obj::integer>(add(start, step));
    return this;
  }

  obj::cons_ptr obj::integer_range::conj(object_ptr const head) const
  {
    return make_box<obj::cons>(head, this);
  }

  native_bool obj::integer_range::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
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
      },
      []() { return false; },
      &o);
  }

  void obj::integer_range::to_string(fmt::memory_buffer &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string obj::integer_range::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string obj::integer_range::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash obj::integer_range::to_hash() const
  {
    return hash::ordered(&base);
  }

  obj::integer_range_ptr obj::integer_range::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto const ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

  size_t obj::integer_range::count() const
  {
    if(is_zero(step))
    {
      throw std::runtime_error("Step shouldn't be 0");
    }

    auto const diff{ sub(start, step) };
    auto const offset{ lt(static_cast<native_integer>(0), step) ? -1 : 1 };

    if((lt(static_cast<native_integer>(0), step)
        && lt(sub(std::numeric_limits<native_integer>::max(),
                  add(step, static_cast<native_integer>(offset))),
              diff))
       || (lt(step, static_cast<native_integer>(0))
           && lt(diff,
                 sub(std::numeric_limits<native_integer>::min(),
                     add(step, static_cast<native_integer>(offset))))))
    {
      throw std::runtime_error("[Integer-Range] Overflow occurred in arithmetic");
    }

    return static_cast<size_t>(div(add(diff + offset, step), step));
  }
}
