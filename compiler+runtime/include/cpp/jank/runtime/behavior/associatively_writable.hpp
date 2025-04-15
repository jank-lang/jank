#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept associatively_writable = requires(T * const t) {
    { t->assoc(object_ref{}, object_ref{}) } -> std::convertible_to<object_ref>;

    { t->dissoc(object_ref{}) } -> std::convertible_to<object_ref>;
  };

  template <typename T>
  concept associatively_writable_in_place = requires(T * const t) {
    { t->assoc_in_place(object_ref{}, object_ref{}) } -> std::convertible_to<object_ref>;

    { t->dissoc_in_place(object_ref{}) } -> std::convertible_to<object_ref>;
  };
}
