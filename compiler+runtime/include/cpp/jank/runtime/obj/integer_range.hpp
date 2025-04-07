#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using integer_ref = jtl::object_ref<struct integer>;
  using cons_ref = jtl::object_ref<struct cons>;
  using integer_range_ref = jtl::object_ref<struct integer_range>;

  /* An integer range from X to Y, exclusive, incrementing by S. */
  /* For non-integer values, use the range object */
  struct integer_range
  {
    static constexpr object_type obj_type{ object_type::integer_range };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    using bounds_check_t = native_bool (*)(integer_ref, integer_ref);

    /* Constructors are only to be used within integer_range.cpp. Prefer integer_range::create. */
    integer_range() = default;
    integer_range(integer_range &&) noexcept = default;
    integer_range(integer_range const &) = default;
    integer_range(integer_ref end);
    integer_range(integer_ref start, obj::integer_ref end);
    integer_range(integer_ref start, obj::integer_ref end, obj::integer_ref step);
    integer_range(integer_ref start,
                  integer_ref end,
                  integer_ref step,
                  bounds_check_t bounds_check);
    //integer_range(integer_ptr start,
    //              integer_ptr end,
    //              integer_ptr step,
    //              bounds_check_t bounds_check,
    //              array_chunk_ptr chunk,
    //              integer_range_ptr chunk_next);

    static object_ptr create(integer_ref end);
    static object_ptr create(integer_ref start, obj::integer_ref end);
    static object_ptr create(integer_ref start, obj::integer_ref end, obj::integer_ref step);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    integer_range_ref seq() const;
    integer_range_ref fresh_seq() const;

    /* behavior::sequenceable */
    integer_ref first() const;
    integer_range_ref next() const;

    /* behavior::sequenceable_in_place */
    integer_range_ref next_in_place();

    /* TODO: behavior::chunkable */
    /* array_chunk_ptr chunked_first() const; */
    /* integer_range_ptr chunked_next() const; */
    /* void force_chunk() const; */

    /* behavior::conjable */
    cons_ref conj(object_ptr head) const;

    /* behavior::metadatable */
    integer_range_ref with_meta(object_ptr m) const;

    /* behavior::countable */
    size_t count() const;

    object base{ object_type::integer_range };
    integer_ref start{};
    integer_ref end{};
    integer_ref step{};
    bounds_check_t bounds_check{};

    /* TODO: behavior::chunkable */
    /* mutable array_chunk_ptr chunk{}; */
    /* mutable integer_range_ptr chunk_next{}; */
    /* mutable integer_range_ptr cached_next{}; */
    jtl::option<object_ptr> meta{};
  };
}
