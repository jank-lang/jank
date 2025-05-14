#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/detail/iterator_sequence.hpp>

namespace jank::runtime::obj
{
  using persistent_sorted_set_ref = oref<struct persistent_sorted_set>;
  using persistent_sorted_set_sequence_ref = oref<struct persistent_sorted_set_sequence>;

  struct persistent_sorted_set_sequence
    : gc
    , obj::detail::iterator_sequence<persistent_sorted_set_sequence,
                                     runtime::detail::native_persistent_sorted_set::const_iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_sorted_set_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    persistent_sorted_set_sequence(persistent_sorted_set_sequence &&) noexcept = default;
    persistent_sorted_set_sequence(persistent_sorted_set_sequence const &) = default;
    using obj::detail::iterator_sequence<
      persistent_sorted_set_sequence,
      runtime::detail::native_persistent_sorted_set::const_iterator>::iterator_sequence;

    object base{ obj_type };
  };
}
