#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  static native_bool positive_step_bounds_check(object_ptr const val, object_ptr const end)
  {
    return lte(end, val);
  }

  static native_bool negative_step_bounds_check(object_ptr const val, object_ptr const end)
  {
    return lte(val, end);
  }

  obj::integer_range::static_object(object_ptr const end)
    : start{ make_box(0) }
    , end{ end }
    , step{ make_box(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  obj::integer_range::static_object(object_ptr const start, object_ptr const end)
    : start{ start }
    , end{ end }
    , step{ make_box(1) }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  obj::integer_range::static_object(object_ptr const start, object_ptr const end, object_ptr const step)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ is_pos(step) ? positive_step_bounds_check : negative_step_bounds_check }
  {
  }

  obj::integer_range::static_object(object_ptr const start,
                            object_ptr const end,
                            object_ptr const step,
                            obj::integer_range::bounds_check_t const bounds_check)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  obj::integer_range::static_object(object_ptr const start,
                            object_ptr const end,
                            object_ptr const step,
                            obj::integer_range::bounds_check_t const bounds_check,
                            obj::array_chunk_ptr const chunk,
                            obj::integer_range_ptr const chunk_next)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
    , chunk{ chunk }
    , chunk_next{ chunk_next }
  {
  }

  object_ptr obj::integer_range::create(object_ptr const end)
  {
    if(is_pos(end))
    {
      return make_box<obj::integer_range>(make_box(0), end, make_box(1), positive_step_bounds_check);
    }
    return obj::persistent_list::empty();
  }

  object_ptr obj::integer_range::create(object_ptr const start, object_ptr const end)
  {
    return create(start, end, make_box(1));
  }

  object_ptr obj::integer_range::create(object_ptr const start, object_ptr const end, object_ptr const step)
  {
    if((is_pos(step) && lt(end, start)) || (is_neg(step) && lt(start, end)) || is_equiv(start, end))
    {
      return obj::persistent_list::empty();
    }
    /* TODO: Repeat object. */
    //else if(is_zero(step))
    //{ return make_box<obj::repeat>(start); }
    return make_box<obj::integer_range>(start,
                                end,
                                step,
                                is_pos(step) ? positive_step_bounds_check
                                             : negative_step_bounds_check);
  }

  obj::integer_range_ptr obj::integer_range::seq()
  {
    return this;
  }

  obj::integer_range_ptr obj::integer_range::fresh_seq() const
  {
    return make_box<obj::integer_range>(start, end, step, bounds_check);
  }

  object_ptr obj::integer_range::first() const
  {
    return start;
  }

  void obj::integer_range::force_chunk() const
  {
    if(chunk)
    {
      return;
    }

    native_vector<object_ptr> arr;
    arr.reserve(chunk_size);
    size_t n{};
    object_ptr val{ start };
    while(n < chunk_size)
    {
      arr.emplace_back(val);
      ++n;
      val = add(val, step);
      if(bounds_check(val, end))
      {
        chunk = make_box<obj::array_chunk>(std::move(arr), static_cast<size_t>(0));
        return;
      }
    }

    if(bounds_check(val, end))
    {
      chunk = make_box<obj::array_chunk>(std::move(arr), static_cast<size_t>(0));
      return;
    }

    chunk = make_box<obj::array_chunk>(std::move(arr), static_cast<size_t>(0));
    chunk_next = make_box<obj::integer_range>(val, end, step, bounds_check);
  }

  obj::integer_range_ptr obj::integer_range::next() const
  {
    if(cached_next)
    {
      return cached_next;
    }

    force_chunk();
    if(chunk->count() > 1)
    {
      auto const smaller_chunk(chunk->chunk_next());
      cached_next = make_box<obj::integer_range>(smaller_chunk->nth(make_box(0)),
                                         end,
                                         step,
                                         bounds_check,
                                         smaller_chunk,
                                         chunk_next);
      return cached_next;
    }
    return chunked_next();
  }

  obj::integer_range_ptr obj::integer_range::next_in_place()
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

  obj::array_chunk_ptr obj::integer_range::chunked_first() const
  {
    force_chunk();
    return chunk;
  }

  obj::integer_range_ptr obj::integer_range::chunked_next() const
  {
    force_chunk();
    if(!chunk_next)
    {
      return nullptr;
    }
    return chunk_next;
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

  void obj::integer_range::to_string(fmt::memory_buffer &buff)
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string obj::integer_range::to_string()
  {
    return runtime::to_string(seq());
  }

  native_persistent_string obj::integer_range::to_code_string()
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
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

  size_t obj::integer_range::count() const
  {
    return 1;
  }
}
