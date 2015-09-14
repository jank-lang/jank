#pragma once

#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/function/argument/call.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    { struct scope; }
  }

  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct native_function_declaration
        {
          std::string name;
          function::argument::type_list<C> arguments;
          type_reference<C> return_type;
          std::function
          <
            C
            (
              std::shared_ptr<interpret::environment::scope> const&,
              function::argument::value_list<C> const&
            )
          > interpret;
          std::shared_ptr<environment::scope> scope;
        };

        template <typename C>
        bool operator <
        (
          native_function_declaration<C> const &lhs,
          native_function_declaration<C> const &rhs
        )
        {
          if(lhs.name < rhs.name)
          { return true; }
          else if(lhs.arguments.size() < rhs.arguments.size())
          { return true; }

          return std::lexicographical_compare
          (
            lhs.arguments.begin(), lhs.arguments.end(),
            rhs.arguments.begin(), rhs.arguments.end(),
            [](auto const &l, auto const &r)
            { return l.name < r.name; }
          );
        }
      }
    }
  }
}
