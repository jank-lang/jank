#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>
#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace interpret
  {
    struct scope
    {
      std::map<std::string, parse::cell::cell> variables;
    };

    parse::cell::cell interpret(std::shared_ptr<scope> const &env, translate::cell::function_body const &root);
    parse::cell::cell interpret(environment::scope &env, parse::cell::list &root);
  }
}
