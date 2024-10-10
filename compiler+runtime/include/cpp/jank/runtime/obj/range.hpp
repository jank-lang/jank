#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  /* A range from X to Y, exclusive, incrementing by S. This is for non-integer values.
   * For integer values, use integer_range. This is not countable in constant time, due
   * to floating point shenanigans. */
  /* TODO: integer_range */
  template <>
  struct static_object<object_type::range> : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_integer chunk_size{ 32 };

    using bounds_check_t = native_bool (*)(object_ptr, object_ptr);

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr end);
    static_object(object_ptr start, object_ptr end);
    static_object(object_ptr start, object_ptr end, object_ptr step);
    static_object(object_ptr start, object_ptr end, object_ptr step, bounds_check_t bounds_check);
    static_object(object_ptr start,
                  object_ptr end,
                  object_ptr step,
                  bounds_check_t bounds_check,
                  obj::array_chunk_ptr chunk,
                  native_box<static_object> chunk_next);

    static object_ptr create(object_ptr end);
    static object_ptr create(object_ptr start, object_ptr end);
    static object_ptr create(object_ptr start, object_ptr end, object_ptr step);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    native_box<static_object> next() const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    /* behavior::chunkable */
    obj::array_chunk_ptr chunked_first() const;
    native_box<static_object> chunked_next() const;
    void force_chunk() const;

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    object base{ object_type::range };
    object_ptr start{};
    object_ptr end{};
    object_ptr step{};
    bounds_check_t bounds_check{};
    mutable obj::array_chunk_ptr chunk{};
    mutable native_box<static_object> chunk_next{};
    mutable native_box<static_object> cached_next{};
    option<object_ptr> meta{};
  };

  namespace obj
  {
    using range = static_object<object_type::range>;
    using range_ptr = native_box<range>;
  }
}
