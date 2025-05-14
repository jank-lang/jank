#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/detail/iterator_sequence.hpp>

namespace jank::runtime::obj
{
  using persistent_hash_set_ref = oref<struct persistent_hash_set>;
  using persistent_hash_set_sequence_ref = oref<struct persistent_hash_set_sequence>;

  struct persistent_hash_set_sequence
    : gc
    , obj::detail::iterator_sequence<persistent_hash_set_sequence,
                                     runtime::detail::native_persistent_hash_set::iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_hash_set_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    persistent_hash_set_sequence(persistent_hash_set_sequence &&) = default;
    persistent_hash_set_sequence(persistent_hash_set_sequence const &) = default;
    using obj::detail::iterator_sequence<
      persistent_hash_set_sequence,
      runtime::detail::native_persistent_hash_set::iterator>::iterator_sequence;

    object base{ obj_type };
  };
}
