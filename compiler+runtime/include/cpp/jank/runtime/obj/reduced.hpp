#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using reduced_ref = oref<struct reduced>;

  struct reduced : object
  {
    static constexpr object_type obj_type{ object_type::reduced };
    static constexpr object_behavior obj_behaviors{ object_behavior::deref };
    static constexpr bool pointer_free{ false };

    reduced(object_ref const o);

    /* behavior::deref */
    object_ref deref() override;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref val{};
  };
}
