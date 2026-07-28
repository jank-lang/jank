#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept conjable = requires(T * const t) {
    { t->conj(object_ref{}) } -> std::convertible_to<object_ref>;
  };

  template <typename T>
  concept conjable_in_place = requires(T * const t) {
    { t->conj_in_place(object_ref{}) } -> std::convertible_to<object_ref>;
  };
}
