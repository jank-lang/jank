#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/interpret/environment/scope.hpp>
#include <jank/interpret/plugin/detail/find_declaration.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace detail
      {
        template <typename... Args>
        void make_function
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope,
          std::string const &name,
          plugin::detail::native_function_definition::function_type const &apply,
          Args &&...args
        )
        {
          auto const &decl
          (
            detail::find_declaration
            (
              trans_scope, name,
              std::forward<Args>(args)...
            )
          );
          /* TODO: emplace. */
          int_scope->native_function_definitions[decl] =
          {
            name,
            [apply](auto const &scope, auto const &args)
            {
              if(args.size() != sizeof...(Args))
              {
                throw expect::error::internal::exception<>
                { "invalid function args" };
              }
              return apply(scope, args);
            }
          };
        }
      }
    }
  }
}
