#pragma once

#include <string>

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
        struct function_call
        {
          function::argument_list arguments;
          std::string name;
        };
      }
    }
  }
}
