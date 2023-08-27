#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept consable = requires(T * const t)
  {
    { t->cons(object_ptr{}) }; // -> consable
  };
}
