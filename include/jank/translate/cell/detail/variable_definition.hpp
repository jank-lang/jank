#pragma once

#include <jank/parse/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        struct variable_definition
        {
          std::string name;
          parse::cell::type type;
        };
      }
    }
  }
}
