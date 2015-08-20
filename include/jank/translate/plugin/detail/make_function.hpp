#pragma once

#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/translate/expect/error/internal/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace detail
      {
        /* Generates parameter names for types. */
        template <typename... Args, std::size_t... Indices>
        function::argument::type_list
        make_args(std::index_sequence<Indices...> const&, Args &&...args)
        {
          return
          { { std::string{ "arg_" } + std::to_string(Indices), args }... };
        }

        /* Defines a native function in the specified scope. */
        template <typename F, typename... Args>
        void make_function
        (
          std::shared_ptr<environment::scope> const &scope,
          std::string const &name,
          F const &apply,
          cell::detail::type_reference const &ret_type,
          Args &&...args
        )
        {
          auto const nested_scope(std::make_shared<environment::scope>());
          nested_scope->parent = scope;

          cell::native_function_definition def
          {
            {
              name,
              make_args(std::index_sequence_for<Args...>{}, args...),
              ret_type,
              [apply](auto const &scope, auto const &args) -> cell::cell
              {
                if(args.size() != sizeof...(Args))
                {
                  throw expect::error::internal::exception<>
                  { "invalid function args" };
                }
                return apply(scope, args);
              },
              nested_scope
            }
          };

          /* TODO: It's easy to overwrite something here; provide an API to register
           * native functions. */
          scope->native_function_definitions[def.data.name].emplace_back
          (std::move(def));
        }
      }
    }
  }
}
