#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell macro
        (
          parse::cell::list const &input,
          std::shared_ptr<scope> const &outer_scope
        );
      }
    }
  }
}
