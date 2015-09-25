#pragma once

#include <map>
#include <stdexcept>
#include <boost/optional.hpp>

#include <jank/translate/environment/special/binding.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/environment/special/apply_expression.hpp>
#include <jank/translate/environment/special/apply_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        boost::optional<cell::cell> apply_all
        (
          parse::cell::list const &list,
          cell::function_body const &translated
        );
      }
    }
  }
}
