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
        inline cell::cell func
        (parse::cell::list const &input, cell::function_body const&)
        {
          auto &data(input.data);
          if(data.empty())
          { throw std::runtime_error{ "invalid parse cell" }; }

          return {};
        }
      }
    }
  }
}
