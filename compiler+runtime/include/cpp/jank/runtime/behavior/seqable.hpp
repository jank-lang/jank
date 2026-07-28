#pragma once

#include <jank/runtime/behavior/conjable.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept seqable = ((T::obj_behaviors & object_behavior::seqable) != object_behavior::none);

  template <typename T>
  concept sequence_like
    = (((T::obj_behaviors & object_behavior::sequence_like) != object_behavior::none)
       && conjable<T>);

  template <typename T>
  concept sequence_like_in_place
    = ((T::obj_behaviors & object_behavior::sequence_like_in_place) != object_behavior::none);
}
