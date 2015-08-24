#pragma once

#include <jank/translate/type/generic/parameter.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        template <typename T>
        struct genericity
        {
          std::vector<parameter<T>> parameters;
        };
      }
    }
  }
}
