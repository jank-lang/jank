#pragma once

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace function
    {
      namespace argument
      {
        namespace detail
        {
          struct argument_value
          {
            std::string name;
            cell::cell cell;
          };
          using value_list = std::vector<argument_value>;
        }
        using value_list = detail::value_list;

        namespace call
        {
          value_list parse
          (
            parse::cell::list const &l,
            environment::scope const &scope
          );
        }
      }
    }
  }
}
