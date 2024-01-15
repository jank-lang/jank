#pragma once

#include <jank/option.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept metadatable = requires(T * const t) {
    {
      t->with_meta(object_ptr{})
    } -> std::convertible_to<object_ptr>;
    {
      t->meta
    } -> std::convertible_to<option<object_ptr>>;
  };

  namespace detail
  {
    object_ptr validate_meta(object_ptr const m);
  }
}
