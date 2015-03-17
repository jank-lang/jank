#pragma once

#include <string>
#include <vector>

#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/expect/type.hpp>
#include <jank/interpret/expect/argument.hpp>
#include <jank/interpret/expect/error/syntax/syntax.hpp>

namespace jank
{
  namespace interpret
  {
    namespace function
    {
      struct argument
      {
        std::string name;
        cell::type type;
      };

      inline std::vector<argument> parse_arguments(cell::list const &list)
      {
        std::vector<argument> ret;

        for(auto it(list.data.begin()); it != list.data.end(); ++it)
        {
          auto const &name(expect::type<cell::type::ident>(*it).data);
          if(++it == list.data.end())
          {
            throw expect::error::syntax::syntax<>
            { "syntax error: expected type after " + name };
          }

          auto const &type(expect::type<cell::type::ident>(*it).data);
          ret.push_back({ name, cell::type_from_string(type) });
        }

        return ret;
      }
    }
  }
}
