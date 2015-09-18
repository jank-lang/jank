#include <jank/translate/expect/error/assertion/exception.hpp>
#include <jank/interpret/plugin/detail/make_function.hpp>
#include <jank/interpret/plugin/assertion/assertion.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace assertion
      {
        void assertion
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          detail::make_function
          (
            trans_scope,
            int_scope,
            "assert",
            [](auto const &scope, auto const &args)
            {
              auto const val
              (
                interpret::expect::type<interpret::cell::type::boolean>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              if(!val)
              { throw translate::expect::error::assertion::exception<>{}; }
              return cell::null{};
            },
            translate::environment::builtin::type::boolean(*trans_scope)
          );
        }
      }
    }
  }
}
