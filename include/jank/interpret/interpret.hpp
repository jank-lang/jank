#pragma once

#include <map>
#include <memory>

#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    /* Says whether or not values from function calls should be returned.
     * By default, any value that isn't from a return statement is ignored.
     * This is used when interpreting a parameter to a function, since the value
     * will be used. Furthermore, in the repl, all values are wanted. */
    enum class consume_style
    {
      normal,
      greedy,
      all
    };

    cell::cell interpret
    (
      std::shared_ptr<environment::scope> const &env,
      translate::cell::function_body const &root,
      consume_style const consume
    );
    cell::cell interpret
    (
      std::shared_ptr<environment::scope> const &env,
      translate::cell::function_body const &root
    );
  }
}
