#pragma once

#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ptr = native_box<struct array_chunk>;
  using cons_ptr = native_box<struct cons>;
  using range_ptr = native_box<struct range>;

  /* A range from X to Y, exclusive, incrementing by S. This is for non-integer values.
   * For integer values, use integer_range. This is not countable in constant time, due
   * to floating point shenanigans. */
  struct range : gc
  {
    static constexpr object_type obj_type{ object_type::range };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };
    static constexpr native_integer chunk_size{ 32 };

    using bounds_check_t = native_bool (*)(object_ptr, object_ptr);

    /* Constructors are only to be used within range.cpp. Prefer range::create. */
    range() = default;
    range(range &&) noexcept = default;
    range(range const &) = default;
    range(object_ptr end);
    range(object_ptr start, object_ptr end);
    range(object_ptr start, object_ptr end, object_ptr step);
    range(object_ptr start, object_ptr end, object_ptr step, bounds_check_t bounds_check);
    range(object_ptr start,
          object_ptr end,
          object_ptr step,
          bounds_check_t bounds_check,
          obj::array_chunk_ptr chunk,
          range_ptr chunk_next);

    static object_ptr create(object_ptr end);
    static object_ptr create(object_ptr start, object_ptr end);
    static object_ptr create(object_ptr start, object_ptr end, object_ptr step);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(util::string_builder &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    range_ptr seq();
    range_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    range_ptr next() const;

    /* behavior::sequenceable_in_place */
    range_ptr next_in_place();

    /* behavior::chunkable */
    obj::array_chunk_ptr chunked_first() const;
    range_ptr chunked_next() const;
    void force_chunk() const;

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    range_ptr with_meta(object_ptr m) const;

    object base{ obj_type };
    object_ptr start{};
    object_ptr end{};
    object_ptr step{};
    bounds_check_t bounds_check{};
    mutable obj::array_chunk_ptr chunk{};
    mutable range_ptr chunk_next{};
    mutable range_ptr cached_next{};
    option<object_ptr> meta{};
  };
}
