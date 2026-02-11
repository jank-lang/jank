#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept realizable = requires(T * const t) {
    { t->is_realized() } -> std::convertible_to<bool>;
  };
}
