#pragma once

#include <jtl/result.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_string_ref = oref<struct persistent_string>;
  using persistent_string_sequence_ref = oref<struct persistent_string_sequence>;

  struct persistent_string : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_string };
    static constexpr bool pointer_free{ false };

    persistent_string() = default;
    persistent_string(persistent_string &&) noexcept = default;
    persistent_string(persistent_string const &) = default;
    persistent_string(jtl::immutable_string const &d);
    persistent_string(jtl::immutable_string &&d);

    static persistent_string_ref empty()
    {
      static auto const ret(make_box<persistent_string>());
      return ret;
    }

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string const &to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(persistent_string const &) const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref key) const;
    bool contains(object_ref key) const;

    /* behavior::indexable */
    object_ref nth(object_ref const index) const;
    object_ref nth(object_ref const index, object_ref const fallback) const;

    jtl::string_result<persistent_string_ref> substring(i64 start) const;
    jtl::string_result<persistent_string_ref> substring(i64 start, i64 end) const;

    /* Returns -1 when not found. Turns the arg into a string, so it accepts anything.
     * Searches for the whole string, not just a char. */
    i64 first_index_of(object_ref const m) const;
    i64 last_index_of(object_ref const m) const;

    /* behavior::countable */
    usize count() const;

    /* behavior::seqable */
    obj::persistent_string_sequence_ref seq() const;
    obj::persistent_string_sequence_ref fresh_seq() const;

    object base{ obj_type };
    jtl::immutable_string data;
  };
}
