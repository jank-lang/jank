#pragma once

#include <string>
#include <stdexcept>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      enum class type
      {
        function_body,
        function_definition,
        function_call,
        type_definition,
        type_reference,
        variable_definition,
        variable_reference,
        literal_value
      };
    }
  }
}
