#pragma once

#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>
#include <jank/runtime/behavior/associatively_writable.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept map_like
    = (countable<T> && associatively_readable<T> && associatively_writable<T> && T::is_map_like);
}
