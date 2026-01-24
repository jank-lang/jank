#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using volatile_ref = oref<struct volatile_>;

  struct volatile_ : object
  {
    static constexpr object_type obj_type{ object_type::volatile_ };
    static constexpr bool pointer_free{ false };

    volatile_();
    volatile_(object_ref const o);

    /* behavior::derefable */
    object_ref deref() const;

    object_ref reset(object_ref const o);

    /*** XXX: Everything here is not thread-safe, but not shared. ***/
    object_ref val{};
  };
}
