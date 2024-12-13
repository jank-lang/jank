#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/detail/iterator_sequence.hpp>

namespace jank::runtime::obj
{
  using persistent_sorted_set_ptr = native_box<struct persistent_sorted_set>;
  using persistent_sorted_set_sequence_ptr = native_box<struct persistent_sorted_set_sequence>;

  struct persistent_sorted_set_sequence
    : gc
    , obj::detail::iterator_sequence<persistent_sorted_set_sequence,
                                     runtime::detail::native_persistent_sorted_set::const_iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_sorted_set_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    persistent_sorted_set_sequence(persistent_sorted_set_sequence &&) noexcept = default;
    persistent_sorted_set_sequence(persistent_sorted_set_sequence const &) = default;
    using obj::detail::iterator_sequence<
      persistent_sorted_set_sequence,
      runtime::detail::native_persistent_sorted_set::const_iterator>::iterator_sequence;

    object base{ object_type::persistent_sorted_set_sequence };
  };
}
