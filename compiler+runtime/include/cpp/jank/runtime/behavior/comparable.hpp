#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept comparable = requires(T * const t) {
    /* Returns how this object compares to the specified object. Comparison, unlike equality,
       * can only be done for objects of the same type. If there's a type mismatch, this function
       * is expected to throw. There are three cases to handle:
       *
       * 1. Comparison should return less than 0 if this object is less than the param
       * 2. Comparison should return 0 if the objects are equal
       * 3. Comparison should return greater than 0 if this object is greater than the param
       *
       * For sequences, all values need to be considered for comparison.
       */
    { t->compare(std::declval<object const &>()) } -> std::convertible_to<native_integer>;
  };
}
