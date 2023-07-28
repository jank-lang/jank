#pragma once

#include <jank/option.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using map = static_object<object_type::map>;
    using map_ptr = native_box<map>;
  }

  namespace behavior
  {
    template <typename T>
    concept metadatable = requires(T * const t)
    {
      { t->with_meta(object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->meta } -> std::convertible_to<option<obj::map_ptr>>;
    };

    namespace detail
    { obj::map_ptr validate_meta(object_ptr const m); }
  }
}
