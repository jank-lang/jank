#pragma once

#include <jank/parse/expect/type.hpp>
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
        template <typename... Args, std::size_t... Indices>
        function::argument::value_list<cell::cell>
        make_args(Args &&...args, std::index_sequence<Indices...> const&)
        { return { { args, std::string{ "arg_" } + std::to_string(Indices) }... }; }

        template <typename F, typename... Args>
        void make_function
        (
          std::shared_ptr<environment::scope> const &scope,
          std::string const &op,
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
              op,
              make_args(args..., std::index_sequence_for<Args...>{}),
              ret_type,
              [apply](auto const &scope, auto const &args) -> cell::cell
              {
                if(args.size() != sizeof...(Args))
                { throw expect::error::internal::exception<>{ "invalid function args" }; }
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
