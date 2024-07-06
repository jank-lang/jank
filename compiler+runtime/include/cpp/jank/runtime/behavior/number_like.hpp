#pragma once

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept number_like = requires(T * const t) {
    { t->to_integer() } -> std::convertible_to<native_integer>;
    { t->to_real() } -> std::convertible_to<native_real>;
  };
}
