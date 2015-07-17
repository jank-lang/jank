#pragma once

#include <jank/translate/cell/detail/type_reference.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        enum class constness
        { non_constant, constant };

        template <typename C>
        struct variable_definition
        {
          std::string name;
          type_reference type;
          constness constant;
          C cell;
        };
      }
    }
  }
}
