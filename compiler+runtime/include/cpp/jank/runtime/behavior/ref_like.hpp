#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept ref_like = requires(T * const t) {
    { t->add_watch(object_ref{}, object_ref{}) } -> std::convertible_to<void>;
    { t->remove_watch(object_ref{}) } -> std::convertible_to<void>;
  };
}
