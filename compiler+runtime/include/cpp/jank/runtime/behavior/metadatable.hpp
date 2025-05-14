#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept metadatable = requires(T * const t) {
    { t->with_meta(object_ref{}) } -> std::convertible_to<object_ref>;
    { t->meta } -> std::convertible_to<jtl::option<object_ref>>;
  };

  namespace detail
  {
    object_ref validate_meta(object_ref const m);
  }
}
