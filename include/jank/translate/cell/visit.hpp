#pragma once

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      /* Follows the same pattern as parse::cell::visit. */
      template <typename Cell, typename Function>
      auto visit(Cell &&c, Function const &func)
      {
        switch(static_cast<type>(c.which()))
        {
          case type::function_body:
            return func(boost::get<function_body>(c));
          case type::function_definition:
            return func(boost::get<function_definition>(c));
          case type::function_call:
            return func(boost::get<function_call>(c));
          default:
            throw std::runtime_error{ "invalid translation cell" };
        }
      }
    }
  }
}
