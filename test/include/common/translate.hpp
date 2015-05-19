#pragma once

#include <string>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>
#include <jank/translate/expect/error/type/overload.hpp>

namespace jank
{
  namespace common
  {
    jank::translate::cell::function_body translate(std::string const &file);
  }
}
