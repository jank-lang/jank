#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ref = oref<struct array_chunk>;
  using cons_ref = oref<struct cons>;
  using range_ptr = oref<struct range>;

  /* A range from X to Y, exclusive, incrementing by S. This is for non-integer values.
   * For integer values, use integer_range. This is not countable in constant time, due
   * to floating point shenanigans. */
  struct range : gc
  {
    static constexpr object_type obj_type{ object_type::range };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };
    static constexpr i64 chunk_size{ 32 };

    using bounds_check_t = bool (*)(object_ref, object_ref);

    /* Constructors are only to be used within range.cpp. Prefer range::create. */
    range() = default;
    range(range &&) noexcept = default;
    range(range const &) = default;
    range(object_ref end);
    range(object_ref start, object_ref end);
    range(object_ref start, object_ref end, object_ref step);
    range(object_ref start, object_ref end, object_ref step, bounds_check_t bounds_check);
    range(object_ref start,
          object_ref end,
          object_ref step,
          bounds_check_t bounds_check,
          obj::array_chunk_ref chunk,
          range_ptr chunk_next);

    static object_ref create(object_ref end);
    static object_ref create(object_ref start, object_ref end);
    static object_ref create(object_ref start, object_ref end, object_ref step);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    range_ptr seq();
    range_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    range_ptr next() const;

    /* behavior::sequenceable_in_place */
    range_ptr next_in_place();

    /* behavior::chunkable */
    obj::array_chunk_ref chunked_first() const;
    range_ptr chunked_next() const;
    void force_chunk() const;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref head) const;

    /* behavior::metadatable */
    range_ptr with_meta(object_ref m) const;

    object base{ obj_type };
    object_ref start{};
    object_ref end{};
    object_ref step{};
    bounds_check_t bounds_check{};
    mutable obj::array_chunk_ref chunk{};
    mutable range_ptr chunk_next{};
    mutable range_ptr cached_next{};
    jtl::option<object_ref> meta{};
  };
}
