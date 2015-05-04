#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp> /* TODO: shouldn't be looking in interpret */

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      /* Ideally used with a generic lambda or struct functor with
       * overloaded op(). This will be called with
       * the extracted cell of its own type; lambdas could condition
       * on is_same<decltype(cell), whatever> if desired. */
      template <typename Cell, typename Func>
      auto visit(Cell &&c, Func const &func)
      {
        switch(static_cast<type>(c.which()))
        {
          case type::boolean:
            return func(boost::get<boolean>(c).data);
          case type::integer:
            return func(boost::get<integer>(c).data);
          case type::real:
            return func(boost::get<real>(c).data);
          case type::string:
            return func(boost::get<string>(c).data);
          case type::ident:
            return func(boost::get<ident>(c).data);
          case type::list:
            return func(boost::get<list>(c).data);
          case type::function:
            return func(boost::get<function>(c).data);
          default:
            throw std::runtime_error{ "invalid parse cell" };
        }
      }
    }
  }
}
