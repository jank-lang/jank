#pragma once

#include <string>
#include <stdexcept>

#include <jank/interpret/expect/error/type/type.hpp>

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
        variable_definition,
        variable_reference,
        literal_value,
        type_definition, /* TODO: integrate with cell and traits */
        type_reference /* TODO: reorder */
      };
    }
  }
}
