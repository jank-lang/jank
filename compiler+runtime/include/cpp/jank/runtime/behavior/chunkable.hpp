#pragma once

#include <jank/runtime/behavior/indexable.hpp>
#include <jank/runtime/behavior/countable.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept chunk_like = requires(T * const t) {
    {
      t->chunk_next()
    } -> std::convertible_to<object_ptr>;

    {
      t->chunk_next_in_place()
    } -> std::convertible_to<object_ptr>;
  } && indexable<T> && countable<T>;

  template <typename T>
  concept chunkable = requires(T * const t) {
    {
      t->chunked_first()
    } -> std::convertible_to<object_ptr>;

    {
      t->chunked_next()
    } -> std::convertible_to<object_ptr>;
  };
}
