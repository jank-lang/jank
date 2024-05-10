#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept derefable = requires(T * const t) {
    {
      t->deref()
    } -> std::convertible_to<object_ptr>;
  };
}
