#include <jank/translate/expect/error/assertion/exception.hpp>
#include <jank/interpret/plugin/detail/find_declaration.hpp>
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
          auto const &decl
          (
            detail::find_declaration
            (
              trans_scope, "assert",
              translate::environment::builtin::type::boolean(*trans_scope)
            )
          );
          int_scope->native_function_definitions[decl] =
          {
            "assert",
            [](auto const &scope, auto const &args) -> cell::cell
            {
              auto const val
              (
                interpret::expect::type<interpret::cell::type::boolean>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              if(!val)
              { throw translate::expect::error::assertion::exception<>{}; }
              return cell::null{};
            }
          };
        }
      }
    }
  }
}
