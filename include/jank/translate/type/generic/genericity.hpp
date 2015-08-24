#pragma once

#include <vector>

#include <boost/variant.hpp>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/type/generic/parameter.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        template <typename C>
        struct genericity
        {
          std::vector<parameter<C>> parameters;
        };
      }
    }
  }
}
