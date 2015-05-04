#pragma once

#include <memory>

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
        struct function_call
        {
          std::string name;
          function::argument::type_list arguments;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
