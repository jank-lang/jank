#pragma once

#include <map>
#include <memory>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace interpret
  {
    struct scope
    {
      /* TODO: Use a custom interpret cell for these values. */
      std::map<std::string, parse::cell::cell> variables;
      std::shared_ptr<scope> parent;

      inline std::experimental::optional<parse::cell::cell> find_variable
      (std::string const &name)
      {
        auto const it(variables.find(name));
        if(it == variables.end())
        {
          if(parent)
          { return parent->find_variable(name); }
          else
          { return {}; }
        }
        return { it->second };
      }
    };

    parse::cell::cell interpret(std::shared_ptr<scope> const &env, translate::cell::function_body const &root);
  }
}
