#pragma once

#include <map>
#include <memory>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    parse::cell::cell interpret
    (
      std::shared_ptr<environment::scope> const &env,
      translate::cell::function_body const &root
    );
  }
}
