#pragma once

#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/conjable.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept set_like = (countable<T> && conjable<T> && T::is_set_like);
}
