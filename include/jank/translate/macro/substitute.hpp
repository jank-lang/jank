#include <jank/translate/translate.hpp>

namespace jank
{
  namespace translate
  {
    namespace macro
    {
      /* For non-macro_calls, this does nothing. */
      template <typename T>
      cell::function_body substitute
      (
        T const &,
        std::shared_ptr<environment::scope> const &,
        cell::function_body &&body
      )
      { return std::move(body); }

      inline cell::function_body substitute
      (
        cell::macro_call const &call,
        std::shared_ptr<environment::scope> const &scope,
        cell::function_body &&body
      )
      {
        return translate
        (
          jtl::it::make_range
          (call.data.result.begin(), call.data.result.end()),
          scope,
          std::move(body)
        );
      }
    }
  }
}
