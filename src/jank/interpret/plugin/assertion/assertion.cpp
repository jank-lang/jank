#include <jank/interpret/plugin/detail/make_function.hpp>
#include <jank/interpret/plugin/assertion/assertion.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>
#include <jank/interpret/expect/error/assertion/exception.hpp>

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
                expect::type<cell::type::boolean>
                (environment::resolve_value(scope, args[0].cell)).data
              );
              if(!val)
              { throw expect::error::assertion::exception<>{}; }
              return cell::null{};
            },
            translate::environment::builtin::type::boolean(*trans_scope)
          );
          detail::make_function
          (
            trans_scope,
            int_scope,
            "assert",
            [](auto const &scope, auto const &args)
            {
              auto const val
              (
                expect::type<cell::type::boolean>
                (environment::resolve_value(scope, args[0].cell)).data
              );
              if(!val)
              {
                auto const err
                (
                  expect::type<cell::type::string>
                  (environment::resolve_value(scope, args[1].cell)).data
                );
                throw expect::error::assertion::exception<>{ err };
              }
              return cell::null{};
            },
            translate::environment::builtin::type::boolean(*trans_scope),
            translate::environment::builtin::type::string(*trans_scope)
          );

          detail::make_function
          (
            trans_scope,
            int_scope,
            "assert-not",
            [](auto const &scope, auto const &args)
            {
              auto const val
              (
                expect::type<cell::type::boolean>
                (environment::resolve_value(scope, args[0].cell)).data
              );
              if(val)
              { throw expect::error::assertion::exception<>{}; }
              return cell::null{};
            },
            translate::environment::builtin::type::boolean(*trans_scope)
          );
          detail::make_function
          (
            trans_scope,
            int_scope,
            "assert-not",
            [](auto const &scope, auto const &args)
            {
              auto const val
              (
                expect::type<cell::type::boolean>
                (environment::resolve_value(scope, args[0].cell)).data
              );
              if(val)
              {
                auto const err
                (
                  expect::type<cell::type::string>
                  (environment::resolve_value(scope, args[1].cell)).data
                );
                throw expect::error::assertion::exception<>{ err };
              }
              return cell::null{};
            },
            translate::environment::builtin::type::boolean(*trans_scope),
            translate::environment::builtin::type::string(*trans_scope)
          );
        }
      }
    }
  }
}
