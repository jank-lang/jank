#pragma once

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      /* Follows the same pattern as parse::cell::visit. */
      template <typename Cell, typename Func, typename Ret>
      auto visit(Cell &&c, Func const &func)
      {
        switch(static_cast<type>(c.which()))
        {
          case type::function_body:
            return func(boost::get<function_body>(c).data);
          case type::function_definition:
            return func(boost::get<function_definition>(c).data);
          case type::function_call:
            return func(boost::get<function_call>(c).data);
          default:
            throw std::runtime_error{ "invalid translation cell" };
        }
      }
    }
  }
}
