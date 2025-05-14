#include <jank/runtime/obj/range.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  static bool positive_step_bounds_check(object_ref const val, object_ref const end)
  {
    return lte(end, val);
  }

  static bool negative_step_bounds_check(object_ref const val, object_ref const end)
  {
    return lte(val, end);
  }

  range::range(object_ref const end)
    : start{ make_box(0) }
    , end{ end }
    , step{ make_box(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  range::range(object_ref const start, object_ref const end)
    : start{ start }
    , end{ end }
    , step{ make_box(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  range::range(object_ref const start, object_ref const end, object_ref const step)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ is_pos(step) ? positive_step_bounds_check : negative_step_bounds_check }
  {
  }

  range::range(object_ref const start,
               object_ref const end,
               object_ref const step,
               range::bounds_check_t const bounds_check)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  range::range(object_ref const start,
               object_ref const end,
               object_ref const step,
               range::bounds_check_t const bounds_check,
               array_chunk_ref const chunk,
               range_ptr const chunk_next)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
    , chunk{ chunk }
    , chunk_next{ chunk_next }
  {
  }

  object_ref range::create(object_ref const end)
  {
    if(is_pos(end))
    {
      return make_box<range>(make_box(0), end, make_box(1), positive_step_bounds_check);
    }
    return persistent_list::empty();
  }

  object_ref range::create(object_ref const start, object_ref const end)
  {
    return create(start, end, make_box(1));
  }

  object_ref range::create(object_ref const start, object_ref const end, object_ref const step)
  {
    if((is_pos(step) && lt(end, start)) || (is_neg(step) && lt(start, end)) || is_equiv(start, end))
    {
      return persistent_list::empty();
    }
    /* TODO: Repeat object. */
    //else if(is_zero(step))
    //{ return repeat::create(start); }
    return make_box<range>(start,
                           end,
                           step,
                           is_pos(step) ? positive_step_bounds_check : negative_step_bounds_check);
  }

  range_ptr range::seq()
  {
    return this;
  }

  range_ptr range::fresh_seq() const
  {
    return make_box<range>(start, end, step, bounds_check);
  }

  object_ref range::first() const
  {
    return start;
  }

  void range::force_chunk() const
  {
    if(chunk.is_some())
    {
      return;
    }

    native_vector<object_ref> arr;
    arr.reserve(chunk_size);
    usize n{};
    object_ref val{ start };
    while(n < chunk_size)
    {
      arr.emplace_back(val);
      ++n;
      val = add(val, step);
      if(bounds_check(val, end))
      {
        chunk = make_box<array_chunk>(std::move(arr), static_cast<size_t>(0));
        return;
      }
    }

    if(bounds_check(val, end))
    {
      chunk = make_box<array_chunk>(std::move(arr), static_cast<size_t>(0));
      return;
    }

    chunk = make_box<array_chunk>(std::move(arr), static_cast<size_t>(0));
    chunk_next = make_box<range>(val, end, step, bounds_check);
  }

  range_ptr range::next() const
  {
    if(cached_next.is_some())
    {
      return cached_next;
    }

    force_chunk();
    if(chunk->count() > 1)
    {
      auto const smaller_chunk(chunk->chunk_next());
      cached_next = make_box<range>(smaller_chunk->nth(make_box(0)),
                                    end,
                                    step,
                                    bounds_check,
                                    smaller_chunk,
                                    chunk_next);
      return cached_next;
    }
    return chunked_next();
  }

  range_ptr range::next_in_place()
  {
    force_chunk();
    if(chunk->count() > 1)
    {
      chunk = chunk->chunk_next_in_place();
      start = chunk->nth(make_box(0));
      return this;
    }
    return chunk_next;
  }

  array_chunk_ref range::chunked_first() const
  {
    force_chunk();
    return chunk;
  }

  range_ptr range::chunked_next() const
  {
    force_chunk();
    if(chunk_next.is_nil())
    {
      return {};
    }
    return chunk_next;
  }

  cons_ref range::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }

  bool range::equal(object const &o) const
  {
    return runtime::sequence_equal(this, &o);
  }

  void range::to_string(util::string_builder &buff)
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string range::to_string()
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string range::to_code_string()
  {
    return runtime::to_code_string(seq());
  }

  uhash range::to_hash() const
  {
    return hash::ordered(&base);
  }

  range_ptr range::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
