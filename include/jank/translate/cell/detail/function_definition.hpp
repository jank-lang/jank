#pragma once

#include <jank/translate/cell/detail/function_body.hpp>
#include <jank/translate/function/argument.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct function_definition
        {
          std::string name;
          function::argument::list arguments;
          function_body<C> body;
        };
      }
    }
  }
}
