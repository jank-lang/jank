#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/interpret/plugin/detail/make_function.hpp>
#include <jank/interpret/plugin/collection/list/list.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace collection
      {
        namespace list
        {
          void list
          (
            std::shared_ptr<translate::environment::scope> const &trans_scope,
            std::shared_ptr<environment::scope> const &int_scope
          )
          {
            detail::make_function
            (
              trans_scope,
              int_scope,
              "list",
              [](auto const &, auto const &)
              { return cell::list{}; }
            );
          }
        }
      }
    }
  }
}
