#pragma once

#include <mutex>

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ref = oref<struct array_chunk>;
  using cons_ref = oref<struct cons>;
  using range_ref = oref<struct range>;

  /* A range from X to Y, exclusive, incrementing by S. This is for non-integer values.
   * For integer values, use integer_range. This is not countable in constant time, due
   * to floating point shenanigans. */
  struct range : object
  {
    static constexpr object_type obj_type{ object_type::range };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };
    static constexpr i64 chunk_size{ 32 };

    using bounds_check_t = bool (*)(object_ref const, object_ref const);

    /* Constructors are only to be used within range.cpp. Prefer range::create. */
    range();
    range(object_ref const end);
    range(object_ref const start, object_ref const end);
    range(object_ref const start, object_ref const end, object_ref const step);
    range(object_ref const start,
          object_ref const end,
          object_ref const step,
          bounds_check_t bounds_check);
    range(object_ref const start,
          object_ref const end,
          object_ref const step,
          bounds_check_t bounds_check,
          obj::array_chunk_ref const chunk,
          range_ref const chunk_next);

    static object_ref create(object_ref const end);
    static object_ref create(object_ref const start, object_ref const end);
    static object_ref create(object_ref const start, object_ref const end, object_ref const step);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::seqable */
    range_ref seq() const;
    range_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    range_ref next() const;

    /* behavior::sequenceable_in_place */
    range_ref next_in_place();

    /* behavior::chunkable */
    obj::array_chunk_ref chunked_first() const;
    range_ref chunked_next() const;
    void force_chunk() const;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head) const;

    /* behavior::metadatable */
    range_ref with_meta(object_ref const m) const;
    object_ref get_meta() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref start{};
    object_ref end{};
    object_ref step{};
    bounds_check_t bounds_check{};
    object_ref meta{};

    /*** XXX: Everything here is thread-safe. ***/
    mutable std::recursive_mutex mutex;
    mutable obj::array_chunk_ref chunk{};
    mutable range_ref chunk_next{};
    mutable range_ref cached_next{};
  };
}
