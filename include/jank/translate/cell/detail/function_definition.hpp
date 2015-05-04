#pragma once

#include <jank/translate/cell/detail/function_body.hpp>
#include <jank/translate/function/argument/definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct function_definition
        {
          std::string name;
          function::argument::type_list arguments;
          function_body<C> body;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
