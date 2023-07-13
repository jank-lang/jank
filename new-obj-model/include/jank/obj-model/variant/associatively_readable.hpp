#pragma once

#include <jank/obj-model/variant/object.hpp>

namespace jank::obj_model::variant
{
  template <typename T>
  concept associatively_readable = requires(T * const t)
  {
    { t->get(object_ptr{}) } -> std::convertible_to<object_ptr>;
    { t->get(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
  };
}
