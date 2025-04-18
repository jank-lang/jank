#pragma once

#include <jank/type.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept number_like = requires(T * const t) {
    { t->to_integer() } -> std::convertible_to<i64>;
    { t->to_real() } -> std::convertible_to<native_real>;
  };
}
