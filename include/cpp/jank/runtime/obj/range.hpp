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

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr const end);
    static_object(object_ptr const start, object_ptr const end);
    static_object(object_ptr const start, object_ptr const end, object_ptr const step);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    native_box<static_object> next() const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    /* behavior::consable */
    obj::cons_ptr cons(object_ptr head) const;

    object base{ object_type::range };
    object_ptr start{};
    object_ptr end{};
    object_ptr step{};
    mutable native_box<static_object> cached_next{};
    /* TODO: Support chunking. */
  };

  namespace obj
  {
    using range = static_object<object_type::range>;
    using range_ptr = native_box<range>;
  }
}
