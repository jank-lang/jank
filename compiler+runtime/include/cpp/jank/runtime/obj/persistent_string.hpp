#pragma once

#include <jank/runtime/object.hpp>
#include <jank/result.hpp>

namespace jank::runtime::obj
{
  using persistent_string_ptr = native_box<struct persistent_string>;
  using persistent_string_sequence_ptr = native_box<struct persistent_string_sequence>;

  struct persistent_string : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_string };
    static constexpr native_bool pointer_free{ false };

    persistent_string() = default;
    persistent_string(persistent_string &&) noexcept = default;
    persistent_string(persistent_string const &) = default;
    persistent_string(native_persistent_string const &d);
    persistent_string(native_persistent_string &&d);

    static persistent_string_ptr empty()
    {
      static auto const ret(make_box<persistent_string>());
      return ret;
    }

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string const &to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(persistent_string const &) const;

    string_result<persistent_string_ptr> substring(native_integer start) const;
    string_result<persistent_string_ptr> substring(native_integer start, native_integer end) const;

    /* Returns -1 when not found. Turns the arg into a string, so it accepts anything.
     * Searches for the whole string, not just a char. */
    native_integer first_index_of(object_ptr const m) const;
    native_integer last_index_of(object_ptr const m) const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::seqable */
    obj::persistent_string_sequence_ptr seq() const;
    obj::persistent_string_sequence_ptr fresh_seq() const;

    object base{ obj_type };
    native_persistent_string data;
  };
}
