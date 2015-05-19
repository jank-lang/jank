#pragma once

#include <string>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace common
  {
    jank::translate::cell::function_body translate(std::string const &file);
  }
}
