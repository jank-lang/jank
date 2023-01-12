#pragma once

#include <cstdlib> // size_t

namespace jank::runtime::behavior
{
  struct countable : virtual gc
  {
    virtual ~countable() = default;

    virtual size_t count() const = 0;
  };
}
