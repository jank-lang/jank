#pragma once

#include <map>
#include <stdexcept>
#include <boost/optional.hpp>

#include <jank/translate/environment/special/function.hpp>
#include <jank/translate/environment/special/lambda.hpp>
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
        boost::optional<cell::cell> apply_definition
        (
          parse::cell::list const &list,
          cell::function_body const &translated
        );
      }
    }
  }
}
