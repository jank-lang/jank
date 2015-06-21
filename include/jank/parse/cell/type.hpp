#pragma once

#include <string>
#include <stdexcept>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      enum class type
      {
        null,
        boolean,
        integer,
        real,
        string,
        ident,
        list,
        comment
      };
    }
  }
}
