#pragma once

#include <jank/runtime/detail/type.hpp>
#include <jank/option.hpp>

namespace jank::runtime
{
  using object_ptr = detail::box_type<struct object>;

  namespace behavior
  {
    struct metadatable
    {
      virtual ~metadatable() = default;

      /* This can't be defined in the base, since it needs to clone the current object.
       * That requires knowledge of the most derived type. */
      virtual object_ptr with_meta(object_ptr const &) const = 0;

      static void validate_meta(object_ptr const &);

      option<object_ptr> meta;
    };
  }
}
