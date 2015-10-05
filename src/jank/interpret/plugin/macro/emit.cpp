#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/macro_primitive.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/plugin/detail/make_function.hpp>
#include <jank/interpret/cell/stream.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace macro
      {
        void emit
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          /* TODO: list version */
          detail::make_function
          (
            trans_scope, int_scope,
            "emit",
            [](auto const &scope, auto const &args)
            {
              /* TODO: Reach out to some global state and give it the value. */
              std::cout << environment::resolve_value(scope, args[0].cell)
                        << std::endl;
              return cell::null{};
            },
            translate::environment::builtin::type::macro_atom(*trans_scope)
          );
        }
      }
    }
  }
}
