#pragma once

#include <vector>
#include <memory>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace function
    {
      namespace ret
      {
        using type_list = std::vector<cell::type_reference>;

        type_list parse
        (
          parse::cell::list const &list,
          std::shared_ptr<environment::scope> const &scope
        );
      }
    }
  }
}
