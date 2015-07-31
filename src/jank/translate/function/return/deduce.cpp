#include <vector>
#include <memory>
#include <algorithm>

#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/function/return/deduce.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        cell::function_body::type deduce(cell::function_body::type body)
        {
          return body;
        }
      }
    }
  }
}
