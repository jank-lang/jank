#pragma once

#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using integer_ptr = native_box<struct integer>;
  using cons_ptr = native_box<struct cons>;
  using integer_range_ptr = native_box<struct integer_range>;

  /* An integer range from X to Y, exclusive, incrementing by S. */
  /* For non-integer values, use the range object */
  struct integer_range
  {
    static constexpr object_type obj_type{ object_type::integer_range };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    using bounds_check_t = native_bool (*)(integer_ptr, integer_ptr);

    /* Constructors are only to be used within integer_range.cpp. Prefer integer_range::create. */
    integer_range() = default;
    integer_range(integer_range &&) noexcept = default;
    integer_range(integer_range const &) = default;
    integer_range(integer_ptr end);
    integer_range(integer_ptr start, obj::integer_ptr end);
    integer_range(integer_ptr start, obj::integer_ptr end, obj::integer_ptr step);
    integer_range(integer_ptr start,
                  integer_ptr end,
                  integer_ptr step,
                  bounds_check_t bounds_check);
    //integer_range(integer_ptr start,
    //              integer_ptr end,
    //              integer_ptr step,
    //              bounds_check_t bounds_check,
    //              array_chunk_ptr chunk,
    //              integer_range_ptr chunk_next);

    static object_ptr create(integer_ptr end);
    static object_ptr create(integer_ptr start, obj::integer_ptr end);
    static object_ptr create(integer_ptr start, obj::integer_ptr end, obj::integer_ptr step);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    integer_range_ptr seq() const;
    integer_range_ptr fresh_seq() const;

    /* behavior::sequenceable */
    integer_ptr first() const;
    integer_range_ptr next() const;

    /* behavior::sequenceable_in_place */
    integer_range_ptr next_in_place();

    /* TODO: behavior::chunkable */
    /* array_chunk_ptr chunked_first() const; */
    /* integer_range_ptr chunked_next() const; */
    /* void force_chunk() const; */

    /* behavior::conjable */
    cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    integer_range_ptr with_meta(object_ptr m) const;

    /* behavior::countable */
    size_t count() const;

    object base{ object_type::integer_range };
    integer_ptr start{};
    integer_ptr end{};
    integer_ptr step{};
    bounds_check_t bounds_check{};

    /* TODO: behavior::chunkable */
    /* mutable array_chunk_ptr chunk{}; */
    /* mutable integer_range_ptr chunk_next{}; */
    /* mutable integer_range_ptr cached_next{}; */
    option<object_ptr> meta{};
  };
}
