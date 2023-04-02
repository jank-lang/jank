#pragma once

#include <jank/option.hpp>

namespace jank::runtime
{
  using object_ptr = native_box<struct object>;

  namespace obj
  { using map_ptr = native_box<struct map>; }

  namespace behavior
  {
    struct metadatable
    {
      virtual ~metadatable() = default;

      /* This can't be defined in the base, since it needs to clone the current object.
       * That requires knowledge of the most derived type. */
      virtual object_ptr with_meta(object_ptr) const = 0;

      static obj::map_ptr validate_meta(object_ptr);

      option<obj::map_ptr> meta;
    };
  }
}
