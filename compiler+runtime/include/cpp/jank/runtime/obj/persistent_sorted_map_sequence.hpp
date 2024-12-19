#pragma once

#include <jank/runtime/obj/detail/base_persistent_map_sequence.hpp>

namespace jank::runtime::obj
{
  using persistent_sorted_map_sequence_ptr = native_box<struct persistent_sorted_map_sequence>;

  struct persistent_sorted_map_sequence
    : detail::base_persistent_map_sequence<
        persistent_sorted_map_sequence,
        runtime::detail::native_persistent_sorted_map::const_iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_sorted_map_sequence };

    using base_persistent_map_sequence::base_persistent_map_sequence;
  };
}
