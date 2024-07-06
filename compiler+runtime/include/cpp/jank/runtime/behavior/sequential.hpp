#pragma once

namespace jank::runtime::behavior
{
  /* Sequential collections are an ordered series of values.
   *
   * This roughly follows Clojure's clojure.lang.Sequential.
   */
  template <typename T>
  concept sequential = T::is_sequential;
}
