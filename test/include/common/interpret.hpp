#pragma once

#include "common/translate.hpp"

#include <jank/interpret/expect/error/assertion/exception.hpp>
#include <jank/interpret/expect/error/type/exception.hpp>

namespace jank
{
  namespace common
  {
    void interpret(std::string const &file);
  }
}
