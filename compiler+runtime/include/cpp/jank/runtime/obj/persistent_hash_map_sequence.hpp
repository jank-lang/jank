#pragma once

#include <jank/runtime/obj/detail/base_persistent_map_sequence.hpp>

namespace jank::runtime::obj
{
  struct persistent_hash_map_sequence
    : detail::base_persistent_map_sequence<
        persistent_hash_map_sequence,
        runtime::detail::native_persistent_hash_map::const_iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_hash_map_sequence };

    using base_persistent_map_sequence::base_persistent_map_sequence;
  };
}
