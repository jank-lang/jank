#pragma once

#include <string>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
#include <jank/translate/expect/error/type/overload.hpp>
#include <jank/translate/expect/error/assertion/exception.hpp>

namespace jank
{
  namespace common
  {
    std::pair<parse::cell::cell, translate::cell::function_body>
    translate(std::string const &file);
  }
}
