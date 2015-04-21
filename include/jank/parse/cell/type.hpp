#pragma once

#include <string>
#include <stdexcept>

#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      enum class type
      {
        boolean,
        integer,
        real,
        string,
        ident,
        list,
        function
      };
    }
  }
}
