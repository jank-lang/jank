#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/detail/iterator_sequence.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_set = static_object<object_type::persistent_set>;
    using persistent_set_ptr = native_box<persistent_set>;
  }

  template <>
  struct static_object<object_type::persistent_set_sequence>
    : gc
    , obj::detail::iterator_sequence<static_object<object_type::persistent_set_sequence>,
                                     runtime::detail::native_persistent_set::iterator>
  {
    static constexpr bool pointer_free{ false };

    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    using obj::detail::iterator_sequence<
      static_object<object_type::persistent_set_sequence>,
      runtime::detail::native_persistent_set::iterator>::iterator_sequence;

    object base{ object_type::persistent_set_sequence };
  };

  namespace obj
  {
    using persistent_set_sequence = static_object<object_type::persistent_set_sequence>;
    using persistent_set_sequence_ptr = native_box<persistent_set_sequence>;
  }
}
