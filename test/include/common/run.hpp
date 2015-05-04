#pragma once

#include <string>

#include <jank/interpret/environment/scope.hpp>
#include <jank/parse/parse.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/prelude/all.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace common
  {
    void run(std::string const &file);
  }
}
