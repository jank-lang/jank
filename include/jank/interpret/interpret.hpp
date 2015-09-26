#pragma once

#include <map>
#include <memory>

#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/consume_style.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
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

    /* Used in the REPL. */
    cell::cell interpret_last
    (
      std::shared_ptr<environment::scope> const &scope,
      translate::cell::function_body root,
      consume_style const consume
    );
  }
}
