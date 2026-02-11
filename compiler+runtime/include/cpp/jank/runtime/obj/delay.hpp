#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using delay_ref = oref<struct delay>;

  struct delay : object
  {
    static constexpr object_type obj_type{ object_type::delay };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    delay();
    delay(object_ref const fn);

    /* behavior::derefable */
    object_ref deref();

    /* behavior::realizable */
    bool is_realized() const;

    /*** XXX: Everything here is thread-safe. ***/
    mutable std::mutex mutex;
    object_ref val{};
    object_ref fn{};
    object_ref error{};
  };
}
