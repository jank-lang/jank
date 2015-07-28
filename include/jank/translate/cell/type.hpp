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
        native_function_definition,
        function_call,
        native_function_call,
        type_definition,
        type_reference,
        binding_definition,
        binding_reference,
        literal_value,
        return_statement,
        if_statement,
        do_statement,
      };
    }
  }
}
