#pragma once

#include <jank/runtime/obj/detail/base_persistent_map_sequence.hpp>
#include <jank/runtime/detail/native_persistent_array_map.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_sequence_ref = jtl::object_ref<struct persistent_array_map_sequence>;

  struct persistent_array_map_sequence
    : detail::base_persistent_map_sequence<
        persistent_array_map_sequence,
        runtime::detail::native_persistent_array_map::const_iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_array_map_sequence };

    using base_persistent_map_sequence::base_persistent_map_sequence;
  };
}
