#pragma once

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
        native_function_declaration,
        function_call,
        indirect_function_call,
        native_function_call,
        function_reference,
        native_function_reference,
        type_definition,
        type_reference,
        binding_definition,
        binding_reference,
        literal_value,
        return_statement,
        if_statement,
        do_statement,
        macro_definition
      };
    }
  }
}
