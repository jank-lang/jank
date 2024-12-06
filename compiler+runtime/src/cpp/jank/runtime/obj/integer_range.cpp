#include "jank/runtime/object.hpp"
#include "jank/runtime/rtti.hpp"
#include "jank/type.hpp"
#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  static native_bool positive_step_bounds_check(native_integer const val, native_integer const end)
  {
    return end <= val;
  }

  static native_bool negative_step_bounds_check(native_integer const val, native_integer const end)
  {
    return val <= end;
  }

  obj::integer_range::static_object(native_integer const end)
    : start{ 0 }
    , end{ end }
    , step{ 1 }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  obj::integer_range::static_object(native_integer const start, native_integer const end)
    : start{ start }
    , end{ end }
    , step{ 1 }
    , bounds_check{ positive_step_bounds_check }
  {
  }

  obj::integer_range::static_object(native_integer const start,
                                    native_integer const end,
                                    native_integer const step)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ step > 0 ? positive_step_bounds_check : negative_step_bounds_check }
  {
  }

  obj::integer_range::static_object(native_integer const start,
                                    native_integer const end,
                                    native_integer const step,
                                    obj::integer_range::bounds_check_t const bounds_check)
    : start{ start }
    , end{ end }
    , step{ step }
    , bounds_check{ bounds_check }
  {
  }

  obj::integer_range::static_object(native_integer const start,
                                    native_integer const end,
                                    native_integer const step,
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

  object_ptr obj::integer_range::create(native_integer const end)
  {
    if(end)
    {
      return make_box<obj::integer_range>(0, end, 1, positive_step_bounds_check);
    }
    return obj::persistent_list::empty();
  }

  object_ptr obj::integer_range::create(native_integer const start, native_integer const end)
  {
    return create(start, end, 1);
  }

  object_ptr obj::integer_range::create(native_integer const start,
                                        native_integer const end,
                                        native_integer const step)
  {
    if((step > 0 && end < start) || (step < 0 && start < end) || start == end)
    {
      return obj::persistent_list::empty();
    }
    /* TODO: Repeat object. */
    //else if(is_zero(step))
    //{ return make_box<obj::repeat>(start); }
    return make_box<obj::integer_range>(start,
                                        end,
                                        step,
                                        step > 0 ? positive_step_bounds_check
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
    return make_box<obj::integer>(start);
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
    auto val{ start };
    while(n < chunk_size)
    {
      arr.emplace_back(make_box<obj::integer>(val));
      ++n;
      val += step;
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
      cached_next
        = make_box<obj::integer_range>(expect_object<obj::integer>(smaller_chunk->buffer[0])->data,
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
      start = (expect_object<obj::integer>(chunk->buffer[0])->data)
        + static_cast<native_integer>(chunk->offset);
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
    if(step == 0)
    {
      throw std::runtime_error("Step shouldn't be 0");
    }

    auto const diff{ end - start };
    auto const offset{ step > 0 ? -1 : 1 };

    return static_cast<size_t>((diff + offset + step) / step);
  }
}
