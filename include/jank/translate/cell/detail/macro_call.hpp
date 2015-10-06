#pragma once

#include <memory>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/detail/macro_definition.hpp>
#include <jank/translate/function/argument/call.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct macro_call
        {
          macro_definition<C> definition;
          function::argument::value_list<C> arguments;
          std::vector<parse::cell::cell> result;
        };
      }
    }
  }
}
