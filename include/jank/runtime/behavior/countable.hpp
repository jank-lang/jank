#pragma once

#include <cstdlib>

namespace jank::runtime::behavior
{
  struct countable
  {
    virtual ~countable() = default;

    virtual size_t count() const = 0;
  };
}
