#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept persistentable = requires(T * const t) {
    {
      t->to_persistent()
    } -> std::convertible_to<object_ptr>;
  };

  template <typename T>
  concept transientable = requires(T * const t) {
    {
      t->to_transient()
    } -> std::convertible_to<object_ptr>;
  };
}
