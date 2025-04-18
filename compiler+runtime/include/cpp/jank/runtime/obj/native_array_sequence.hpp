#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using native_array_sequence_ref = oref<struct native_array_sequence>;
  using cons_ref = oref<struct cons>;

  struct native_array_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::native_array_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    native_array_sequence() = delete;
    native_array_sequence(native_array_sequence &&) noexcept = default;
    native_array_sequence(native_array_sequence const &) = default;
    native_array_sequence(object_ref * const arr, usize const size);
    native_array_sequence(object_ref * const arr, usize const index, usize const size);

    template <typename... Args>
    native_array_sequence(object_ref const first, Args const... rest)
      : arr{ make_array_box<object_ref>(first, rest...) }
      , size{ sizeof...(Args) + 1 }
    {
    }

    /* behavior::object_like */
    bool equal(object const &o) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    native_array_sequence_ref seq();
    native_array_sequence_ref fresh_seq();

    /* behavior::countable */
    usize count() const;

    /* behavior::sequence */
    object_ref first() const;
    native_array_sequence_ref next() const;
    obj::cons_ref conj(object_ref head);

    /* behavior::sequenceable_in_place */
    native_array_sequence_ref next_in_place();

    object base{ obj_type };
    jtl::ptr<object_ref> arr{};
    usize index{};
    usize size{};
  };
}
