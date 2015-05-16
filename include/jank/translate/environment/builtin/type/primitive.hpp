#pragma once

#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace builtin
      {
        namespace type
        {
          std::shared_ptr<scope> add_primitives(std::shared_ptr<scope> const &s);
        }
      }
    }
  }
}
