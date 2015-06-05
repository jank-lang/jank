#include <jank/translate/environment/special/constant.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell constant
        (parse::cell::list const &input, cell::function_body const &body)
        { return detail::variable::make(input, body, cell::detail::constness::constant); }
      }
    }
  }
}
