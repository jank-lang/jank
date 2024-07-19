#pragma once

#include <cstdlib> // size_t

namespace jank::runtime::behavior
{
  template <typename T>
  concept countable = requires(T * const t) {
    { t->count() } -> std::convertible_to<size_t>;
  };
}
