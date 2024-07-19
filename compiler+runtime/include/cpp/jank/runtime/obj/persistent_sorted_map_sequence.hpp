#pragma once

#include <jank/runtime/obj/detail/base_persistent_map_sequence.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::persistent_sorted_map_sequence>
    : obj::detail::base_persistent_map_sequence<
        object_type::persistent_sorted_map_sequence,
        runtime::detail::native_persistent_sorted_map::const_iterator>
  {
    using base_persistent_map_sequence::base_persistent_map_sequence;
  };

  namespace obj
  {
    using persistent_sorted_map_sequence
      = static_object<object_type::persistent_sorted_map_sequence>;
    using persistent_sorted_map_sequence_ptr = native_box<persistent_sorted_map_sequence>;
  }
}
