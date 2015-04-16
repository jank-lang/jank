#pragma once

#include <stdexcept>

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
        inline cell::cell func(parse::cell::list const &input, cell::cell output)
        {
          auto &data(input.data);
          if(data.empty())
          { throw std::runtime_error{ "invalid parse cell" }; }

          return output;
        }
      }
    }
  }
}
