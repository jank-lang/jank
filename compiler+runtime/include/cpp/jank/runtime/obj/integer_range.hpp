#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime
{
  /* An integer range from X to Y, exclusive, incrementing by S. */
  /* For non-integer values, use the range object */
  template <>
  struct static_object<object_type::integer_range> : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    using bounds_check_t = native_bool (*)(obj::integer_ptr, obj::integer_ptr);

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(obj::integer_ptr end);
    static_object(obj::integer_ptr start, obj::integer_ptr end);
    static_object(obj::integer_ptr start, obj::integer_ptr end, obj::integer_ptr step);
    static_object(obj::integer_ptr start,
                  obj::integer_ptr end,
                  obj::integer_ptr step,
                  bounds_check_t bounds_check);
    static_object(obj::integer_ptr start,
                  obj::integer_ptr end,
                  obj::integer_ptr step,
                  bounds_check_t bounds_check,
                  obj::array_chunk_ptr chunk,
                  native_box<static_object> chunk_next);

    static object_ptr create(obj::integer_ptr end);
    static object_ptr create(obj::integer_ptr start, obj::integer_ptr end);
    static object_ptr create(obj::integer_ptr start, obj::integer_ptr end, obj::integer_ptr step);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<static_object> seq() const;
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    obj::integer_ptr first() const;
    native_box<static_object> next() const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    /* TODO: behavior::chunkable */
    /* obj::array_chunk_ptr chunked_first() const; */
    /* native_box<static_object> chunked_next() const; */
    /* void force_chunk() const; */

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    /* behavior::countable */
    size_t count() const;

    object base{ object_type::integer_range };
    obj::integer_ptr start{};
    obj::integer_ptr end{};
    obj::integer_ptr step{};
    bounds_check_t bounds_check{};

    /* TODO: behavior::chunkable */
    /* mutable obj::array_chunk_ptr chunk{}; */
    /* mutable native_box<static_object> chunk_next{}; */
    /* mutable native_box<static_object> cached_next{}; */
    option<object_ptr> meta{};
  };

  namespace obj
  {
    using integer_range = static_object<object_type::integer_range>;
    using integer_range_ptr = native_box<integer_range>;
  }
}
