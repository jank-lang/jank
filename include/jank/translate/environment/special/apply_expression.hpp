#pragma once

#include <map>
#include <stdexcept>
#include <experimental/optional>

#include <jank/translate/environment/special/function.hpp>
#include <jank/translate/environment/special/if_statement.hpp>
#include <jank/translate/environment/special/do_statement.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        std::experimental::optional<cell::cell> apply_expression
        (
          parse::cell::list const &list,
          cell::function_body const &translated
        );
      }
    }
  }
}
