#pragma once

#include <map>
#include <stdexcept>
#include <experimental/optional>

#include <jank/translate/environment/special/function.hpp>
#include <jank/translate/environment/special/binding.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/environment/special/apply_expression.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        std::experimental::optional<cell::cell> apply_all
        (
          parse::cell::list const &list,
          cell::function_body const &translated
        );
      }
    }
  }
}
