#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/obj/detail/iterator_sequence.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = oref<struct persistent_list>;
  using persistent_list_sequence_ref = oref<struct persistent_list_sequence>;

  /* TODO: We need to get rid of this. In Clojure, a list *is* a sequence. */
  struct persistent_list_sequence
    : gc
    , obj::detail::iterator_sequence<persistent_list_sequence,
                                     runtime::detail::native_persistent_list::iterator>
  {
    static constexpr object_type obj_type{ object_type::persistent_list_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    persistent_list_sequence() = default;
    persistent_list_sequence(persistent_list_sequence &&) noexcept = default;
    persistent_list_sequence(persistent_list_sequence const &) = default;
    using obj::detail::iterator_sequence<
      persistent_list_sequence,
      runtime::detail::native_persistent_list::iterator>::iterator_sequence;

    /* behavior::metadatable */
    persistent_list_sequence_ref with_meta(object_ref const m) const
    {
      auto const meta(behavior::detail::validate_meta(m));
      auto ret(make_box<persistent_list_sequence>(coll, begin, end, size));
      ret->meta = meta;
      return ret;
    }

    object base{ obj_type };
    jtl::option<object_ref> meta;
  };
}
